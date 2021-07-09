#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define DEFEALT_LINE_NUM 9
#define BUFF_SIZE 1024
#define PBUFF_SIZE 32

static int line_num = DEFEALT_LINE_NUM;
static char* fpath;

typedef struct buffer {
	char data[BUFF_SIZE];
	int frist;
	int line_num;
} buffer_type;

void print_error(const char* s) {
	fprintf(stderr, "error in %s", s);
	exit(1);
}


//根据\n判断行数,存储每行字数,
struct buffer* fill_buf_last(int fd, struct buffer* buf) {
	if (read(fd, buf->data, BUFF_SIZE) > 0) {
		int num_tmp = 0;	//某一行的char数量
		int line_tmp = 0;	//到第几行的标志
		for (int i = BUFF_SIZE -1; i >= 0; --i) {
			++num_tmp;
			if (buf->data[i] == '\n') {
				--line_num;
				++line_tmp;
				if (line_num == 0) {
					buf->frist = i;
					return buf;
				}
			}
		}
		return buf;
	}
	else
		print_error("read");
}
char* tail() {
	int fd;
	int offset = 0;
	int cur_line = 0;
	char buf[BUFF_SIZE];
	if ((fd = open(fpath, O_RDONLY)) == -1) {
		print_error("open");
	}
	buffer_type* buf_list[PBUFF_SIZE];
	int pbuf_cur = 0;
	if (lseek(fd, 0, SEEK_END) == -1)
		print_error("seek");
	while (line_num > 0) {
		if (lseek(fd, - BUFF_SIZE * 2, SEEK_CUR) != -1) {
			buffer_type* buf = (buffer_type*)malloc(sizeof(buffer_type));
			fill_buf_last(fd, buf);
			buf_list[pbuf_cur++] = buf;
			if (pbuf_cur == PBUFF_SIZE ) {
				break;
			}
		}
		else
			print_error("seek");
	}
	--pbuf_cur;
	for (; pbuf_cur >= 0; --pbuf_cur) {
		printf("%s", buf_list[pbuf_cur]->data + buf_list[pbuf_cur]->frist);
		free(buf_list[pbuf_cur]);
	}
	printf("\n");
	if (close(fd) == -1)
		print_error("close");
	
}


int main(int ac, char* av[]) {
	if (ac < 2)
		perror("too little param");
	fpath = av[1];
	if (ac > 2 && strcmp(av[1], "n")) {
		line_num = atoi( av[2] );
		fpath = av[3];
	}
	tail();
}