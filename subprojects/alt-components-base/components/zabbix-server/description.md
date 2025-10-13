Provides Zabbix server — the central process of Zabbix software.

The server performs the polling and trapping of data, it calculates triggers, sends notifications to users.
It is the central component to which Zabbix agents and proxies report data on availability and integrity of systems.
The server can itself remotely check networked services (such as web servers and mail servers) using simple service checks.

The server is the central repository in which all configuration, statistical and operational data is stored,
and it is the entity in Zabbix that will actively alert administrators when problems arise in any of the monitored systems.

The functioning of a basic Zabbix server is broken into three distinct components they are: 
* Zabbix server
* web frontend
* database storage

All of the configuration information for Zabbix is stored in the database, which both the server and the web frontend interact with.
For example, when you create a new item using the web frontend (or API) it is added to the items table in the database.
Then, about once a minute Zabbix server will query the items table for a list of the items which are active
that is then stored in a cache within the Zabbix server.
This is why it can take up to two minutes for any changes made in Zabbix frontend to show up in the latest data section.

**ALT Linux Wiki:** <https://www.altlinux.org/Zabbix>

**Home page:** <https://www.zabbix.com/>
