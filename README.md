# UnixDev
	1. fork-execv exit-wait
	(1)子线程exit后发送信号SIGCHLD,父进程wait接受信号,获得传来的返回值.
	(2)通过重定向输出输入通信:tinybc.
	2. Environ
	使用
	extern char** environ;
	获得全局系统变量,添加或修改值传递信息:mysh
	3. Pipe
	通过重定向pipe的io进行进程通讯:mypipe
	4. Kill-signal
	kill发送信号: tkserv::ticket_reclaim()
	5. Internet socket
	(1)SOCK_STREAM:tinyweb/twebserv
	(2)SOCK_DGRAM:dgramserv
	6. Named socket/unix socket
	类似文件io
	7. File&Locks in local
	Fcntl : unixdev
	Select/poll : unixdev::select_demo()
	8. Share memory
	Shmget -> shmat -> shmctl : shmtserv
	9. Semaphore
	Semget -> semat -> semop -> semctl : shmtserv
	10. NFS
	Network file sys
	11. CGI服务器
	高效的半同步/半反应堆多线程池多进程池,统一事件源,轮询;每个子线程处理多个connect
	12.http服务器
	半同步/半反应堆多线程池,主线程发送事件到线程队列处理,timechecker in minheap
	
	
	

