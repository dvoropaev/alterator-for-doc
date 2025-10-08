Provides an Alterator module called “Certificate Authority”, which allows management of SSL certificates used to secure connections between network nodes.

To ensure a secure connection for the client (for example, a web browser as the client software), the key issue is whether the certificate is accepted.

The following scenarios are possible when accepting a certificate:

* the server certificate is signed by a certificate authority (CA) known to the client
* the server certificate is signed by a CA unknown to the client
* the server certificate is self-signed

**ALT Linux Wiki:** <https://www.altlinux.org/Alterator-ca>
