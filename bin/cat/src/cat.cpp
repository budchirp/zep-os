extern "C" {
    void print(const char* str);
    long long read_file(const char* path, char* buffer, unsigned long long size, unsigned long long offset);
    void exit(int code);
}

extern "C" int main(int argc, char** argv) {
    if (argc < 2 || argv[1] == nullptr) {
        print("Usage: cat <filename>\n");
        exit(-1);
    }

    char buf[1024];
    long long read_bytes = read_file(argv[1], buf, sizeof(buf) - 1, 0);
    if (read_bytes < 0) {
        print("Failed to read file (Permission Denied or Not Found).\n");
        exit(-1);
    }

    buf[read_bytes] = '\0';
    print(buf);
    print("\n");

    exit(0);
    return 0;
}
