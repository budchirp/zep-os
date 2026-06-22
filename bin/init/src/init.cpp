extern "C" {
    void print(const char* str);
    unsigned int spawn(const char* path, const char** argv);
    void exit(int code);
}

extern "C" int main() {
    print("Init process started (root).\n");
    print("Spawning /System/Binaries/login...\n");

    unsigned int login_pid = spawn("/System/Binaries/login", nullptr);
    if (login_pid == 0) {
        print("Failed to spawn login!\n");
        while (true) {}
    }

    print("Init exiting to pass control to login.\n");
    exit(0);
    return 0;
}
