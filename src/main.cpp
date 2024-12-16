#include <fstream>
#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <vector>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "char_checker.h"

class TaskProducer {
public:
    TaskProducer(std::string_view fileData)
        : _fileData(fileData)
        , _current(fileData.data()) {}

    std::optional<std::string_view> getTask() {
        std::lock_guard lock(_mutex);

        auto* fileEnd = &_fileData.back();
        if (_current > fileEnd) {
            return {};
        }

        auto begin = _current;
        auto sizeLeft = fileEnd - _current + 1;
        auto taskSize = std::min<size_t>(sizeLeft, 4096 * 10);

        auto end = begin + taskSize;
        while (end <= fileEnd && IsTartgetChar(*end)) {
            ++end;
        }

        _current = end;
        return std::string_view(begin, end - begin);
    }

    void publishRes(std::unordered_map<std::string, int> freqMap) {
        std::lock_guard lock(_mutex);
        _totalRes.merge(freqMap);
        for (auto const& [ch, count] : freqMap) {
            _totalRes[ch] += count;
        }
    }

    std::unordered_map<std::string, int> _totalRes;
    std::string_view _fileData;
    const char* _current{};
    std::mutex _mutex;
};

static void _threadFunc(TaskProducer* producer) {
    std::unordered_map<std::string, int> freqMap;

    for (;;) {
        auto task = producer->getTask();
        if (!task) {
            break;
        }

        std::string_view::iterator bit1 = task->begin();
        std::string_view::iterator bit2 = task->end();
        while (bit1 != bit2) {
            auto it1 = GetTargetIterator(bit1, bit2);
            auto it2 = GetNonTargetIterator(it1 + 1, bit2);
            if (it1 != bit2 && it1 != it2) {
                auto word = ToLowerCase(it1, it2);
                freqMap[word]++;
            }
            bit1 = it2;
        }
    }

    producer->publishRes(std::move(freqMap));
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file_path> <output_file_path>" << std::endl;
        return 1;
    }

    const char* inputFilePath = argv[1];

    int fd = open(inputFilePath, O_RDONLY);
    if (fd == -1) {
        std::cerr << "Can't open file: " << argv[1] << std::endl;
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        std::cerr << "Can't read input file stat: " << argv[1] << std::endl;
        close(fd);
        return 1;
    }

    const char* outFilePath = argv[2];

    std::ofstream out;
    out.open(outFilePath);
    if (!out.is_open()) {
        std::cerr << "Can't create output file: " << argv[2] << std::endl;
        close(fd);
        return 1;
    }

    size_t fileSize = sb.st_size;

    unsigned char* fileData = static_cast<unsigned char*>(mmap(nullptr, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
    if (fileData == MAP_FAILED) {
        std::cerr << "Error while mmap file: " << argv[1] << std::endl;
        close(fd);
        return 1;
    }

    TaskProducer producer(std::string_view{ (const char*)fileData, fileSize });

    std::vector<std::thread> threads;
    for (int ind = 0; ind < 10; ++ind) {
        threads.emplace_back(_threadFunc, &producer);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    munmap(fileData, fileSize);
    close(fd);

    std::vector<std::pair<int, std::string>> sortedRes;
    sortedRes.reserve(producer._totalRes.size());
    for (auto const& [word, count] : producer._totalRes) {
        sortedRes.emplace_back(count, word);
    }
    std::sort(sortedRes.begin(), sortedRes.end(), [](std::pair<int, std::string> a, std::pair<int, std::string> b) {
        return a.first > b.first || (a.first == b.first && a.second < b.second);
    });
    for (auto const& [count, word] : sortedRes) {
        out << word << " " << count << std::endl;
    }

    return 0;
}
