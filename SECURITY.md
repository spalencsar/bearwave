# Security Policy

BearWave is currently in a public beta phase. Security reports are very welcome and should be handled carefully, especially since BearWave processes external internet radio streams and metadata.

## Official Distribution & Security Notice

We only guarantee the security and integrity of our official distribution channels:
1. **Our official Flatpak repository** (`https://flatpak.bearwave.app/`), which is GPG-signed by the author.
2. **Our official AUR package** (`bearwave-git`), where the source code is cloned directly from our official GitHub repository and built locally on your machine.

We **do not verify, support, or guarantee** the security of any other third-party binary repositories (such as unofficial repositories on the openSUSE Build Service, private arch repositories, or other third-party package mirrors). Installing from unofficial sources carries security risks, as the binaries are not compiled or controlled by the original author.

## Supported Versions

During the beta phase, security fixes target the latest code on `main` and the most recent published release when practical.

Older releases are not guaranteed to receive backported fixes. Users should update to the latest available release after a security fix is published.

## Reporting a Vulnerability

Please **do not** open a public issue for a vulnerability before it has been reviewed.

Preferred reporting path:

1. Use GitHub's private vulnerability reporting / security advisory feature for this repository, if available.
2. If private reporting is not available, contact the maintainer through GitHub and request a private disclosure channel.

Include as much detail as possible:

- Affected BearWave version or commit SHA
- Operating system and desktop environment
- Steps to reproduce the issue
- Potential impact of the vulnerability
