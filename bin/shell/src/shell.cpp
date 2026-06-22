extern "C" {
    void print(const char* str);
    char read_char();
    unsigned int spawn(const char* path, const char** argv);
    int kill(unsigned int pid);
    long long read_file(const char* path, char* buffer, unsigned long long size, unsigned long long offset);
    long long write_file(const char* path, const char* buffer, unsigned long long size, unsigned long long offset);
    unsigned int getuid();

    // Shared libc functions
    int strcmp(const char* s1, const char* s2);
    int strncmp(const char* s1, const char* s2, unsigned long long n);
    unsigned long long strlen(const char* s);
    unsigned int parse_int(const char* str);
    void print_char(char c);
    void readline(char* buf, int max_len);
}

extern "C" int main() {
    print("\n=================================\n");
    print("Welcome to Zep OS Interactive Shell\n");
    print("=================================\n\n");

    char cmd[256];
    char file_buf[1024];

    while (true) {
        print("zep> ");
        readline(cmd, sizeof(cmd));

        if (strcmp(cmd, "help") == 0) {
            print("Available commands:\n");
            print("  help                           - Show this help message\n");
            print("  whoami                         - Print current user identity\n");
            print("  cat <path>                     - Print contents of a file\n");
            print("  spawn <path>                   - Spawn a process\n");
            print("  kill <pid>                     - Kill a process by PID\n");
            print("  useradd <username> <uid>       - Add a new user (root only)\n");
        } else if (strcmp(cmd, "whoami") == 0) {
            unsigned int pid = spawn("/System/Binaries/whoami", nullptr);
            if (pid == 0) {
                print("Failed to run whoami.\n");
            }
        } else if (strncmp(cmd, "cat ", 4) == 0) {
            const char* path = cmd + 4;
            const char* cat_argv[3];
            cat_argv[0] = "/System/Binaries/cat";
            cat_argv[1] = path;
            cat_argv[2] = nullptr;
            unsigned int pid = spawn("/System/Binaries/cat", cat_argv);
            if (pid == 0) {
                print("Failed to run cat.\n");
            }
        } else if (strncmp(cmd, "spawn ", 6) == 0) {
            const char* path = cmd + 6;
            const char* spawn_argv[2];
            spawn_argv[0] = path;
            spawn_argv[1] = nullptr;
            unsigned int pid = spawn(path, spawn_argv);
            if (pid == 0) {
                print("Failed to spawn process.\n");
            } else {
                print("Process spawned successfully. PID: ");
                char pbuf[16];
                int pidx = 0;
                unsigned int temp = pid;
                while (temp > 0) {
                    pbuf[pidx++] = '0' + (temp % 10);
                    temp /= 10;
                }
                for (int i = pidx - 1; i >= 0; --i) {
                    print_char(pbuf[i]);
                }
                print("\n");
            }
        } else if (strncmp(cmd, "kill ", 5) == 0) {
            unsigned int pid = parse_int(cmd + 5);
            int res = kill(pid);
            if (res < 0) {
                print("Failed to kill process (Permission Denied or Not Found).\n");
            } else {
                print("Process killed successfully.\n");
            }
        } else if (strncmp(cmd, "useradd ", 8) == 0) {
            const char* args = cmd + 8;
            const char* space = nullptr;
            for (const char* p = args; *p; ++p) {
                if (*p == ' ') {
                    space = p;
                    break;
                }
            }
            if (space == nullptr) {
                print("Usage: useradd <username> <uid>\n");
            } else {
                char username[64];
                unsigned long long u_len = static_cast<unsigned long long>(space - args);
                if (u_len > 63) {
                    u_len = 63;
                }
                for (unsigned long long i = 0; i < u_len; ++i) {
                    username[i] = args[i];
                }
                username[u_len] = '\0';

                const char* uid_str = space + 1;
                unsigned int uid = parse_int(uid_str);

                long long read_bytes = read_file("/Config/users", file_buf, sizeof(file_buf) - 200, 0);
                if (read_bytes < 0) {
                    print("Failed to read user database (Permission Denied).\n");
                } else {
                    file_buf[read_bytes] = '\0';

                    unsigned long long f_len = strlen(file_buf);
                    unsigned long long u_name_len = strlen(username);
                    for (unsigned long long i = 0; i < u_name_len; ++i) {
                        file_buf[f_len++] = username[i];
                    }
                    file_buf[f_len++] = ':';

                    char uid_buf[16];
                    int uid_idx = 0;
                    unsigned int temp = uid;
                    if (temp == 0) {
                        uid_buf[uid_idx++] = '0';
                    } else {
                        while (temp > 0) {
                            uid_buf[uid_idx++] = '0' + (temp % 10);
                            temp /= 10;
                        }
                    }
                    for (int i = uid_idx - 1; i >= 0; --i) {
                        file_buf[f_len++] = uid_buf[i];
                    }
                    file_buf[f_len++] = '\n';
                    file_buf[f_len] = '\0';

                    long long write_bytes = write_file("/Config/users", file_buf, f_len, 0);
                    if (write_bytes < 0) {
                        print("Failed to update user database (Permission Denied).\n");
                    } else {
                        print("User added successfully.\n");
                    }
                }
            }
        } else if (strlen(cmd) > 0) {
            print("Unknown command: ");
            print(cmd);
            print("\nType 'help' for assistance.\n");
        }
    }
    return 0;
}
