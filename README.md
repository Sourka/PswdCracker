# PswdCracker
This simple CLI program, written for educational purposes, uses brute–force method to guess password provided by user, and displays how much time it took. 

It takes characters from password, which are expected to occur in it, then passes it to function trying to reconstruct password. Also, the program takes hints about minimal and maximal password length.

If password has whitespaces (but not a newline), it's treated as a passphrase. In that case, not characters, but words are used.  
