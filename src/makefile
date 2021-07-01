#!make
.PHONY : reinstall uninstall install clean main
include /etc/os-release

main:
	gcc \
		-D LINUX -D AMD64 \
		$(shell $(shell apxs -q APR_CONFIG) --cflags --includes) \
		$(shell python3-config --cflags --includes) \
		-fPIC -DSHARED_MODULE \
		-I$(shell apxs -q INCLUDEDIR) $(shell apxs -q CFLAGS) \
		-c mod_jinja2.c \
		$(shell python3-config --libs);

	gcc \
		-D LINUX -D AMD64 \
		$(shell $(shell apxs -q APR_CONFIG) --link-ld) \
		$(shell python3-config --ldflags) \
		-shared -o mod_jinja2.so mod_jinja2.o \
		$(shell python3-config --libs);

clean:
	rm -f mod_jinja2.o;
	rm -f mod_jinja2.so;


install:
ifeq (${ID_LIKE}, debian)
	cp -f mod_jinja2.so /usr/lib/apache2/modules/;
	chmod 644 /usr/lib/apache2/modules/mod_jinja2.so;
	chown root:root /usr/lib/apache2/modules/mod_jinja2.so;

	cp -f resources/debian/mod_jinja2.conf /etc/apache2/mods-available/;
	chmod 644 /etc/apache2/mods-available/mod_jinja2.conf;
	chown root:root /etc/apache2/mods-available/mod_jinja2.conf;

	cp -f resources/debian/mod_jinja2.load /etc/apache2/mods-available/;
	chmod 644 /etc/apache2/mods-available/mod_jinja2.load;
	chown root:root /etc/apache2/mods-available/mod_jinja2.load;
	
	rm -f /etc/apache2/mods-enabled/mod_jinja2.conf;
	rm -f /etc/apache2/mods-enabled/mod_jinja2.load;
	ln -s ../mods-available/mod_jinja2.conf /etc/apache2/mods-enabled/mod_jinja2.conf;
	ln -s ../mods-available/mod_jinja2.load /etc/apache2/mods-enabled/mod_jinja2.load;

	@echo 'Remember validate configuration and restart apache server.';

else ifeq (${ID_LIKE}, "rhel fedora")
	cp -f mod_jinja2.so /etc/httpd/modules/;
	chown root:root /etc/httpd/modules/mod_jinja2.so;
	semanage fcontext -a -t httpd_modules_t /etc/httpd/modules/mod_jinja2.so;
	restorecon -rv /etc/httpd/modules/mod_jinja2.so;

	cp -f resources/rhel/15-mod_jinja2.conf /etc/httpd/conf.modules.d/;
	chmod 600 /etc/httpd/conf.modules.d/15-mod_jinja2.conf;
	chown root:root /etc/httpd/conf.modules.d/15-mod_jinja2.conf;

	@echo 'Remember validate configuration and restart httpd server.';

else
	@echo "OS/Distribution is not supported: ${ID_LIKE}";

endif


uninstall:
ifeq (${ID_LIKE}, debian)
	rm -f /etc/apache2/mods-enabled/mod_jinja2.conf;
	rm -f /etc/apache2/mods-enabled/mod_jinja2.load;
	rm -f /etc/apache2/mods-available/mod_jinja2.conf;
	rm -f /etc/apache2/mods-available/mod_jinja2.load;
	rm -f /usr/lib/apache2/modules/mod_jinja2.so;
	@echo 'Remember restart apache server.';

else ifeq (${ID_LIKE}, "rhel fedora")
	rm -f /etc/httpd/modules/mod_jinja2.so;
	rm -f /etc/httpd/conf.modules.d/15-mod_jinja2.conf;

	@echo 'Remember restart httpd server.';

else
	@echo "OS/Distribution is not supported.";

endif


reinstall: uninstall install