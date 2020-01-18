# big-jim-poker-bot
A program that plays heads up no limit hold'em.

Big Jim is designed to keep track of all the possible hands he and his opponent could have in a given situation, while updating and continually making decisions based on these ranges. Most of the decision making is done arbitrarily based on simulating Jim's hand versus his opponent's range of hands, or by seeing how his own range of hands compares to his opponent's.

# PlayBigJim.cpp
This is a version of Big Jim that allows the user to play against Big Jim in the terminal.

# BigJim9Max.cpp
This is a version of Big Jim designed to be able to play at a table of up to 8 opponents. This version is much slower than the others because Jim has to keep track of all of his opponents' ranges all at once.

# BigJimLionsDen.cpp
This version of Big Jim is designed to be compatible with my other project, The Lion's Den, which is a program designed to let 2 heads up poker bots compete against each other by communicating through text files.
