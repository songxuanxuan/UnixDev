#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void set_up();
int get_ticket();
int release_ticket();
void shut_down();
int check_validate();

int main() {
	set_up();
	if (get_ticket() == -1)
		exit(1);

	printf("working stage 1...\n");
	sleep(6);
	if (check_validate() == -1)
		if (get_ticket() != 0)		
			exit(1);
	printf("working stage 2...\n");
	sleep(3);


	release_ticket();
	shut_down();

}