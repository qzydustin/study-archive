# Run tips

- First run oracle.sql to create the databases.
- Run server by parameters. >java -jar server.jar [database.username] [database.password]
- Run admin/client/display by parameters. >java -jar administrator/client/display.jar [server.ip]
    if without parameters, default is localhost
- Server must be started before other subsystems
- Close admin/client/display by "X".
- Admin need login first, or it will show "login first".(There is a admin account (admin:admin) in database)
  