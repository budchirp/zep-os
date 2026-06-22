extern "C" {
    void print(const char* str);
    void readline(char* buf, int max_len);
    int strcmp(const char* s1, const char* s2);
    int setuid(unsigned int uid);
    unsigned int spawn(const char* path, const char** argv);
    long long read_file(const char* path, char* buffer, unsigned long long size, unsigned long long offset);
    void exit(int code);
}

static unsigned int parse_int(const char* str) {
    unsigned int val = 0;
    while (*str >= '0' && *str <= '9') {
        val = val * 10 + (*str - '0');
        str++;
    }
    return val;
}

extern "C" int main() {
    char username[64];
    char users_buf[1024];

    print("\n=================================\n");
    print("Welcome to Zep OS\n");
    print("=================================\n");

    while (true) {
        print("Login: ");
        readline(username, sizeof(username));

        // Trim newline/cr if any
        int len = 0;
        while (username[len] != '\0') {
            len++;
        }
        if (len > 0 && username[len - 1] == '\n') {
            username[len - 1] = '\0';
            len--;
        }
        if (len > 0 && username[len - 1] == '\r') {
            username[len - 1] = '\0';
            len--;
        }

        if (len == 0) {
            continue;
        }

        // Read /Config/users
        long long read_bytes = read_file("/Config/users", users_buf, sizeof(users_buf) - 1, 0);
        if (read_bytes < 0) {
            print("Error: Could not read user database.\n");
            continue;
        }
        users_buf[read_bytes] = '\0';

        // Parse line by line
        int idx = 0;
        bool found = false;
        unsigned int found_uid = 0;

        while (idx < read_bytes) {
            // Extract a line
            char line[128];
            int l_idx = 0;
            while (idx < read_bytes && users_buf[idx] != '\n' && l_idx < 127) {
                line[l_idx++] = users_buf[idx++];
            }
            if (idx < read_bytes && users_buf[idx] == '\n') {
                idx++;
            }
            line[l_idx] = '\0';

            // Find colon
            int colon_idx = -1;
            for (int i = 0; i < l_idx; ++i) {
                if (line[i] == ':') {
                    colon_idx = i;
                    break;
                }
            }

            if (colon_idx != -1) {
                line[colon_idx] = '\0';
                if (strcmp(line, username) == 0) {
                    found_uid = parse_int(line + colon_idx + 1);
                    found = true;
                    break;
                }
            }
        }

        if (found) {
            if (setuid(found_uid) != 0) {
                print("Error: Failed to set UID.\n");
            } else {
                print("Switching identity. Spawning shell...\n");
                unsigned int shell_pid = spawn("/System/Binaries/shell", nullptr);
                if (shell_pid == 0) {
                    print("Error: Failed to spawn shell.\n");
                } else {
                    exit(0);
                }
            }
        } else {
            print("Login incorrect\n");
        }
    }
    return 0;
}
