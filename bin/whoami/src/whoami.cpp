extern "C" {
    void print(const char* str);
    unsigned int getuid();
    long long read_file(const char* path, char* buffer, unsigned long long size, unsigned long long offset);
    void print_char(char c);
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
    unsigned int uid = getuid();

    char users_buf[1024];
    long long read_bytes = read_file("/Config/users", users_buf, sizeof(users_buf) - 1, 0);
    if (read_bytes >= 0) {
        users_buf[read_bytes] = '\0';

        int idx = 0;
        while (idx < read_bytes) {
            char line[128];
            int l_idx = 0;
            while (idx < read_bytes && users_buf[idx] != '\n' && l_idx < 127) {
                line[l_idx++] = users_buf[idx++];
            }
            if (idx < read_bytes && users_buf[idx] == '\n') {
                idx++;
            }
            line[l_idx] = '\0';

            int colon_idx = -1;
            for (int i = 0; i < l_idx; ++i) {
                if (line[i] == ':') {
                    colon_idx = i;
                    break;
                }
            }

            if (colon_idx != -1) {
                unsigned int entry_uid = parse_int(line + colon_idx + 1);
                if (entry_uid == uid) {
                    line[colon_idx] = '\0';
                    print(line);
                    print("\n");
                    exit(0);
                    return 0;
                }
            }
        }
    }

    // Fallback: print numerical UID
    print("unknown (UID ");
    char ubuf[16];
    int uidx = 0;
    unsigned int temp = uid;
    if (temp == 0) {
        ubuf[uidx++] = '0';
    } else {
        while (temp > 0) {
            ubuf[uidx++] = '0' + (temp % 10);
            temp /= 10;
        }
    }
    for (int i = uidx - 1; i >= 0; --i) {
        print_char(ubuf[i]);
    }
    print(")\n");

    exit(0);
    return 0;
}
