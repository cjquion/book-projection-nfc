# book-projection-nfc

Install:
follow install directions for libnfc: https://github.com/nfc-tools/libnfc
pip install PyQt5
pip install python-vlc

compile c code:
gcc -o nfc_poll nfc_poll.c -lnfc

to run:
- get nfc tag UIDs, and record them in byte format in nfc_poll.c,
and add memcmp code blocks to run when the UID is matched.

- compile

- make sure nfc reader is attached (RS-S300 Felica Sony in my case)
- ./nfc_poll
- tap tags
