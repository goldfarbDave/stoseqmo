#pragma once
#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
std::vector<byte_t> open_file_as_bytes(std::string path) {
    std::ifstream stream(path, std::ios::in | std::ios::binary);
    if (stream.fail()) {
        std::ostringstream ss;
        ss << "Failed to open: " << path;
        throw std::runtime_error{ss.str()};
    }
    // Using transform as fill to meander around type funny business
    std::vector<byte_t> ar;
    std::transform(std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{},
                   std::back_inserter(ar),
                   [](auto const &el) {
                       return static_cast<byte_t>(el);
                   });
    return ar;
}

struct MemoryContents {
    std::vector<byte_t> bytes;
    std::vector<bit_t> bits;
};

MemoryContents load_file_in_memory(std::string path ) {
    auto bytes = open_file_as_bytes(path);
    auto bits = bytevec_to_bitvec(bytes);
    return MemoryContents{bytes, bits};
}

const std::map<std::string, std::string> cantbry_name_to_path = {
    {"alice29.txt",  ROOT_PATH"/data/cantrbry/alice29.txt"},
    {"asyoulik.txt", ROOT_PATH"/data/cantrbry/asyoulik.txt"},
    {"cp.html",      ROOT_PATH"/data/cantrbry/cp.html"},
    {"fields.c",     ROOT_PATH"/data/cantrbry/fields.c"},
    {"grammar.lsp",  ROOT_PATH"/data/cantrbry/grammar.lsp"},
    {"kennedy.xls",  ROOT_PATH"/data/cantrbry/kennedy.xls"},
    {"lcet10.txt",   ROOT_PATH"/data/cantrbry/lcet10.txt"},
    {"plrabn12.txt", ROOT_PATH"/data/cantrbry/plrabn12.txt"},
    {"ptt5",         ROOT_PATH"/data/cantrbry/ptt5"},
    {"sum",          ROOT_PATH"/data/cantrbry/sum"},
    {"xargs.1",      ROOT_PATH"/data/cantrbry/xargs.1"},
};
const std::vector<std::string> cantbry_names = {
    "alice29.txt",
    "asyoulik.txt",
    "cp.html",
    "fields.c",
    "grammar.lsp",
    "kennedy.xls",
    "lcet10.txt",
    "plrabn12.txt",
    "ptt5",
    "sum",
    "xargs.1",
};
const std::map<std::string, std::string> calgary_name_to_path = {
    {"bib",    ROOT_PATH"/data/calgary/bib"},
    {"book1",  ROOT_PATH"/data/calgary/book1"},
    {"book2",  ROOT_PATH"/data/calgary/book2"},
    {"geo",    ROOT_PATH"/data/calgary/geo"},
    {"news",   ROOT_PATH"/data/calgary/news"},
    {"obj1",   ROOT_PATH"/data/calgary/obj1"},
    {"obj2",   ROOT_PATH"/data/calgary/obj2"},
    {"paper1", ROOT_PATH"/data/calgary/paper1"},
    {"paper2", ROOT_PATH"/data/calgary/paper2"},
    {"paper3", ROOT_PATH"/data/calgary/paper3"},
    {"paper4", ROOT_PATH"/data/calgary/paper4"},
    {"paper5", ROOT_PATH"/data/calgary/paper5"},
    {"paper6", ROOT_PATH"/data/calgary/paper6"},
    {"pic",    ROOT_PATH"/data/calgary/pic"},
    {"progc",  ROOT_PATH"/data/calgary/progc"},
    {"progl",  ROOT_PATH"/data/calgary/progl"},
    {"progp",  ROOT_PATH"/data/calgary/progp"},
    {"trans",  ROOT_PATH"/data/calgary/trans"},
};
const std::vector<std::string> calgary_names = {
    "bib",
    // "book1",
    // "book2",
    // "geo",
    // "news",
    // "obj1",
    // "obj2",
    // "paper1",
    // "paper2",
    // "paper3",
    // "paper4",
    // "paper5",
    // "paper6",
    // "pic",
    // "progc",
    // "progl",
    "progp",
    "trans",
};
