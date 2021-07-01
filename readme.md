# Jinja2 module for Apache httpd

## Tested on

GNU/Linux distributions:

- Ubuntu Server 20.04 LTS.
- CentOS 7.


## Requirements

- Debian o Rhel distribution based.
- Python3.
- Jinja2 module for python3.
- Apache httpd evn vars for load python3 library.


## Installation

Compile module and install it using makefile:

```bash
root@server:~# make;
root@server:~# make install;
```

Create enviroment var:

On Debian based, edit `/etc/apache2/envvars` and write:
`export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libpython3.8.so`.

On Rhel based, edit `/etc/sysconfig/httpd` and write
`export LD_PRELOAD=/usr/lib64/libpython3.so`.

For find python3 library on other systems you can use:
`find / -iname "libpython3*"`.

For validate installation before restart use `apachectl configtest`:

```bash
root@server:~# apachectl configtest;
Syntax OK
```

And, restart server:

For Debian based use `systemctl restart apache2`.

For Rhel based use  `systemctl restart httpd`.


## Uninstallation

Use makefile:

```bash
root@server:~# make uninstall;
```