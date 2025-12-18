#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define VERSION "0.1.0"

typedef struct {
    int show_numbers;          // -n
    int show_nonprint;         // -v
    int squeeze_blank;         // -s
    int show_ends;             // -e
    int show_tabs;             // -t
    int show_all;              // -A
    int number_nonblank;       // -b
    int help;                  // --help
    int version;               // --version
} CatOptions;

void print_help(const char *progname) {
    printf("Usage: %s [OPTION]... [FILE]...\n", progname);
    printf("Concatenate FILE(s) to standard output.\n\n");
    printf("  -A, --show-all           equivalent to -vET\n");
    printf("  -b, --number-nonblank    number nonempty output lines, overrides -n\n");
    printf("  -e                       equivalent to -vE\n");
    printf("  -E, --show-ends          display $ at end of each line\n");
    printf("  -n, --number             number all output lines\n");
    printf("  -s, --squeeze-blank      suppress repeated empty output lines\n");
    printf("  -t                       equivalent to -vT\n");
    printf("  -T, --show-tabs          display TAB characters as ^I\n");
    printf("  -v, --show-nonprinting   use ^ and M- notation, except for LFD and TAB\n");
    printf("      --help               display this help and exit\n");
    printf("      --version            output version information and exit\n");
}

void print_version(void) {
    printf("cat_clone %s\n", VERSION);
}

// Вывод одного символа с обработкой -v
void output_char(int c, const CatOptions *opts) {
    if (opts->show_all || opts->show_nonprint) {
        if (c == '\t' || c == '\n') {
            putchar(c);
            return;
        }
        if (c >= 32 && c < 127) {
            putchar(c);
        } else if (c == '\n') {
            putchar('$');
            putchar('\n');
        } else if (c < 0) {
            printf("M-");
            c += 256;
            if (c >= 32 && c < 127) {
                putchar(c);
            } else {
                printf("^%c", c ^ 64);
            }
        } else {
            printf("^%c", c ^ 64);
        }
    } else {
        putchar(c);
    }
}

// Вывод одной строки с учётом всех опций
void process_line(char *line, size_t len, int *line_num, int *prev_blank,
                  const CatOptions *opts) {
    int is_blank = (len == 1 && line[0] == '\n') || (len == 0);

    // -s: подавление повторяющихся пустых строк
    if (opts->squeeze_blank && is_blank && *prev_blank) {
        return;
    }
    *prev_blank = is_blank;

    // Нумерация строк
    if (opts->number_nonblank && !is_blank) {
        printf("%6d\t", (*line_num)++);
    } else if (opts->show_numbers) {
        printf("%6d\t", (*line_num)++);
    }

    // Вывод содержимого строки с учётом -v, -t, -e и т.д.
    for (size_t i = 0; i < len; ++i) {
        int c = (unsigned char)line[i];

        if (opts->show_tabs && c == '\t') {
            printf("^I");
        } else if (opts->show_ends && c == '\n') {
            putchar('$');
            putchar('\n');
        } else {
            output_char(c, opts);
        }
    }
}

// Чтение и обработка одного файла (или stdin при NULL)
void cat_file(const char *filename, CatOptions *opts, int *line_num) {
    FILE *fp;
    if (filename == NULL || strcmp(filename, "-") == 0) {
        fp = stdin;
    } else {
        fp = fopen(filename, "r");
        if (!fp) {
            fprintf(stderr, "cat: %s: %s\n", filename, strerror(errno));
            return;
        }
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int prev_blank = 0;

    while ((read = getline(&line, &len, fp)) != -1) {
        process_line(line, (size_t)read, line_num, &prev_blank, opts);
    }

    free(line);
    if (fp != stdin) fclose(fp);
}

int main(int argc, char *argv[]) {
    CatOptions opts = {0};
    int opt;

    // Парсинг аргументов вручную
    for (int i = 1; i < argc; ++i) {
        char *arg = argv[i];

        if (strcmp(arg, "--help") == 0) {
            opts.help = 1;
        } else if (strcmp(arg, "--version") == 0) {
            opts.version = 1;
        } else if (strcmp(arg, "-A") == 0 || strcmp(arg, "--show-all") == 0) {
            opts.show_all = 1;
            opts.show_nonprint = 1;
            opts.show_ends = 1;
            opts.show_tabs = 1;
        } else if (strcmp(arg, "-b") == 0 || strcmp(arg, "--number-nonblank") == 0) {
            opts.number_nonblank = 1;
        } else if (strcmp(arg, "-e") == 0) {
            opts.show_nonprint = 1;
            opts.show_ends = 1;
        } else if (strcmp(arg, "-E") == 0 || strcmp(arg, "--show-ends") == 0) {
            opts.show_ends = 1;
        } else if (strcmp(arg, "-n") == 0 || strcmp(arg, "--number") == 0) {
            opts.show_numbers = 1;
        } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--squeeze-blank") == 0) {
            opts.squeeze_blank = 1;
        } else if (strcmp(arg, "-t") == 0) {
            opts.show_nonprint = 1;
            opts.show_tabs = 1;
        } else if (strcmp(arg, "-T") == 0 || strcmp(arg, "--show-tabs") == 0) {
            opts.show_tabs = 1;
        } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--show-nonprinting") == 0) {
            opts.show_nonprint = 1;
        } else if (arg[0] == '-') {
            fprintf(stderr, "cat: invalid option -- '%s'\n", arg + 1);
            fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
            return 1;
        }
    }

    if (opts.help) {
        print_help(argv[0]);
        return 0;
    }
    if (opts.version) {
        print_version();
        return 0;
    }

    // Если флаги -b и -n указаны вместе, -b имеет приоритет
    if (opts.number_nonblank && opts.show_numbers) {
        opts.show_numbers = 0;
    }

    // Сбор имён файлов
    int file_count = 0;
    char **files = NULL;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] != '-') {
            file_count++;
        }
    }

    if (file_count == 0) {
        // Чтение из stdin
        int line_num = 1;
        cat_file(NULL, &opts, &line_num);
    } else {
        files = malloc(file_count * sizeof(char *));
        int idx = 0;
        for (int i = 1; i < argc; ++i) {
            if (argv[i][0] != '-') {
                files[idx++] = argv[i];
            }
        }

        int line_num = 1;
        for (int i = 0; i < file_count; ++i) {
            cat_file(files[i], &opts, &line_num);
        }

        free(files);
    }

    return 0;
}
