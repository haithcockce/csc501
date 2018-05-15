default:
	sudo yum localinstall ./fuse-python-0.2.1-1.el6.x86_64.rpm -y &> /dev/null
	gcc postmark-1.51.c -o postmark

debug:
	sudo yum localinstall ./fuse-python-0.2.1-1.el6.x86_64.rpm -y
	sudo yum install python-pyroute2-0.3.3-1.el6.noarch ipython python-setuptools-0.6.10-3.el6.noarch -y
	sudo easy_install ipdb
	gcc postmark-1.51.c -o postmark

clean:
	rm postmark 
