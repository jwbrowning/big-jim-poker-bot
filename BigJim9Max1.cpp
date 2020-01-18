#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <string>
#include <cmath>
#include <list>
#include <random>
#include <cmath>

using namespace std;

//Struct for a given hand, holds info about the hand
typedef struct Hand {
	int myCards[2][2];
	int flop[3][2];
	int turn[1][2];
	int river[1][2];
	int position,potSize,stage;
	bool aggressor;
	int ranges[9][1326][4];
	int numBets;
	int playerToAct;
	int activePlayers[9];
	int callSizes[9];
	int numActivePlayers;
	int numPlayers;
	int smallBlind;
	int bigBlind;
	int actionsLeft;
	//Constructor, sets defaults for beginning of a hand
	Hand(int small=0,int big=0,int nuPlayers=9) {
		int deck[52][2] = {
			{2,1},{3,1},{4,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},{11,1},{12,1},{13,1},{14,1},
			{2,2},{3,2},{4,2},{5,2},{6,2},{7,2},{8,2},{9,2},{10,2},{11,2},{12,2},{13,2},{14,2},
			{2,3},{3,3},{4,3},{5,3},{6,3},{7,3},{8,3},{9,3},{10,3},{11,3},{12,3},{13,3},{14,3},
			{2,4},{3,4},{4,4},{5,4},{6,4},{7,4},{8,4},{9,4},{10,4},{11,4},{12,4},{13,4},{14,4},
		};
		bigBlind=big;
		smallBlind=small;
		numPlayers=nuPlayers;
		actionsLeft=numPlayers;
		numActivePlayers=0;
		for(int i=0;i<9;i++) {
			activePlayers[i]=0;
		}
		for(int i=0;i<numPlayers;i++) {
			activePlayers[i]=1;
		}
		for(int i=0;i<9;i++) {
			if(activePlayers[i]==1) {
				numActivePlayers++;
			}
		}
		if(numPlayers>3) {
			playerToAct=3;
		} else {
			playerToAct=0;
		}
		stage=0;
		numBets=0;
		potSize=big+small;
		for(int i=0;i<numPlayers;i++) {
			if(i==1) {
				callSizes[i]=big-small;
			} else if (i==2) {
				callSizes[i]=0;
			} else {
				callSizes[i]=big;
			}
		}
		if(numPlayers==2) {
			callSizes[0]=big-small;
			callSizes[1]=0;
		}
		int n=52,r=2;
		vector<bool> v(n);
		int j=0;
		int k=0;
	    fill(v.end() - r, v.end(), true);
	    do {
	        for (int i=0;i<n;i++) {
	            if (v[i]) {
	            	for(int l=0;l<9;l++) {
	            		ranges[l][j][k]=deck[i][0];
	            		ranges[l][j][k+1]=deck[i][1];
	            	}
	            	k+=2;
	            }
	        }
	        j++;
	        k=0;
	    } while (next_permutation(v.begin(), v.end()));

	    for(int i=0;i<6;i++) {
	    	flop[i/2][i%2]=0;
	    }for(int i=0;i<2;i++) {
	    	turn[i/2][i%2]=0;
	    }for(int i=0;i<2;i++) {
	    	river[i/2][i%2]=0;
	    }
	}
} Hand;

//Method Headers/Prototypes and Descriptions:

//updateRange: removes the every hand in range1 that has an equity inside or outside the range (eL,eU) against range2
void updateRange(float eL, float eU, bool keepInside, int (*ranges)[9][1326][4], int deck[52][2],Hand& hand);
//foldEquity: calculates the percentage of hands in range1 that have an equity of <e against range2
float foldEquity(float e, int (*ranges)[9][1326][4], int deck[52][2],Hand& hand);
//potential: returns whether or not the hand has potential, ie it can hit a flush or straight on a later street
bool potential(int hand[2][2],Hand& h);
//charToInt: used for converting user input into readable card values
int charToInt(char c);
//hey im a comment
float convertedEquity(int numPlayers, float equity);
//resetRange: resets range of hands to starting range, used as safety for when a range is narrowed to 0 hands
void resetRange(int (*range)[1326][4],int deck[52][2]);
//strengthOfHand: returns a float (the superior variable type) representing a 5 card hand's strength from Straight flush to high card
float strengthOfHand(int hand[5][2]);
//action: called when it's Jim's turn to act, makes Jim perform an action and print to console
void action(int deck[52][2],Hand& hand,int *myStack,int big);
//simHands: simulates a hand against a range a given amount of times to find the hand's winrate, can also find a range's winrate against another range
float simHands(int myCards[52][2],int n,int deck[n][2],int trials,Hand& hand,int (*o)[9][1326][4],int (*m)[1326][4],bool a);
//Fold: action function for when Jim folds
void Fold(int *myStack,Hand& hand);
//Check: for when Jim checks
void Check(int *myStack,Hand& hand,int deck[52][2]);
//Call: for when Jim calls
void Call(int *myStack,Hand& hand,int deck[52][2]);
//Raise: for when Jim raises
void Raise(int *myStack,Hand& hand,int value,int deck[52][2]);


//main method for user commands and main management of game
int main() {

	//declare and initialize variables for the game

	int deck[52][2] = {
		{2,1},{3,1},{4,1},{5,1},{6,1},{7,1},{8,1},{9,1},{10,1},{11,1},{12,1},{13,1},{14,1},
		{2,2},{3,2},{4,2},{5,2},{6,2},{7,2},{8,2},{9,2},{10,2},{11,2},{12,2},{13,2},{14,2},
		{2,3},{3,3},{4,3},{5,3},{6,3},{7,3},{8,3},{9,3},{10,3},{11,3},{12,3},{13,3},{14,3},
		{2,4},{3,4},{4,4},{5,4},{6,4},{7,4},{8,4},{9,4},{10,4},{11,4},{12,4},{13,4},{14,4},
	};

	int stackSize;
	int small,big;
	int numPlayers;
	string command;

	/*
	Stage:
	0 - Preflop
	1 - Flop
	2 - Turn
	3 - River
	*/

	//Get key information from user:
	cout<<"How many players?\n";
	cin>>numPlayers;
	cout<<"What's my stack size?\n";
	cin>>stackSize;
	int myStack=stackSize;
	cout<<"Small and big blind?\n";
	cin>>small>>big;

	//loop for while game is in progress
	bool playing=true;
	while(playing) {

		//declare hand variable for each hand
		Hand hand = Hand(small,big,numPlayers);

		//Display information to user:
		cout<<"Me: "<<myStack<<endl;
		cout<<"Pot: "<<hand.potSize<<", "<<hand.callSizes[hand.position]<<" to call\n";
		//get position, 0=dealer
		cout<<"What's my position?\n";
		cin>>hand.position;
		//Set stacks according to who is which blind
		if(numPlayers>2) {
			if(hand.position==1) {
				myStack-=small;
			} else if (hand.position==2) {
				myStack-=big;
			}
		} else if (numPlayers==2) {
			if(hand.position==0) {
				myStack-=small;
			} else if (hand.position==1) {
				myStack-=big;
			}
		}
		//Get Jim's starting hand
		cout<<"What's my hand?\n";
		char info[4] = {' ',' ',' ',' '};
		cin>>info[0]>>info[1]>>info[2]>>info[3];
		for(int i=0;i<4;i++) {
			hand.myCards[i/2][i%2]=charToInt(info[i]);
		}

		char hold[6];
		//loop for while hand is taking place:
		bool handInProgress=true;
		while(handInProgress) {
			//loop to make sure user enters valid command
			bool validCommand=false;
			while(!validCommand) {
				validCommand=true;

				while(hand.playerToAct>=numPlayers || hand.activePlayers[hand.playerToAct]==0) {
					hand.playerToAct++;
					if(hand.playerToAct>=numPlayers) {
						hand.playerToAct=0;
					}
				}

				int numActivePlayers=0;
				for(int i=0;i<numPlayers;i++) {
					if(hand.activePlayers[i]==1) {
						numActivePlayers++;
					}
				}

				//display info to user and ask for command
				cout<<"Me: "<<myStack<<endl;
				cout<<"Pot: "<<hand.potSize<<", "<<hand.callSizes[hand.playerToAct]<<" to call\n";

				if(numActivePlayers==1 && hand.activePlayers[hand.position]==1) {
					command="win";
				} else if (hand.activePlayers[hand.position]==0) {
					command="lose";
				} else if (hand.playerToAct==hand.position && hand.actionsLeft>0) {
					cout<<"My turn..."<<endl;
					command="act";
				} else {
					if(hand.actionsLeft>0) {
						switch(hand.playerToAct) {
							case 0:
								cout<<"BUT's turn...";
								break;
							case 1:
								if(numPlayers>2) {
									cout<<"SB's turn...";
								} else if(numPlayers==2) {
									cout<<"BB's turn...";
								}
								break;
							case 2:
								cout<<"BB's turn...";
								break;
							case 3:
								cout<<"UTG's turn...";
								break;
							case 4:
								if(numPlayers==5) {
									cout<<"CO's turn...";
								} else {
									cout<<"UTG+1's turn...";
								}
								break;
							case 5:
								if(numPlayers==6) {
									cout<<"CO's turn...";
								} else if (numPlayers==7) {
									cout<<"HJ's turn...";
								} else {
									cout<<"UTG+2's turn...";
								}
								break;
							case 6:
								if(numPlayers==7) {
									cout<<"CO's turn...";
								} else if (numPlayers==8) {
									cout<<"HJ's turn...";
								} else {
									cout<<"UTG+3's turn...";
								}
								break;
							case 7:
								if(numPlayers==8) {
									cout<<"CO's turn...";
								} else {
									cout<<"HJ's turn...";
								}
								break;
							case 8:
								cout<<"CO's turn...";
								break;

						}
						cout<<endl;
					} else {
						switch(hand.stage) {
							case 0:
								cout<<"Waiting for flop...";
								break;
							case 1:
								cout<<"Waiting for turn...";
								break;
							case 2:
								cout<<"Waiting for river...";
								break;
							case 3:
								cout<<"Showdown.";
								break;
						}
						cout<<endl;
					}
					cin>>command;
				}

				//flop command - for inputing the flop cards
				if (command=="flop") {
					cin>>hold[0]>>hold[1]>>hold[2]>>hold[3]>>hold[4]>>hold[5];
					for(int i=0;i<6;i++) {
						hand.flop[i/2][i%2]=charToInt(hold[i]);
					}
					hand.stage=1;
					hand.numBets=0;
					hand.playerToAct=1;
					hand.actionsLeft=hand.numActivePlayers;
				//turn command - for inputing the turn card
				} else if (command=="turn") {
					cin>>hold[0]>>hold[1];
					for(int i=0;i<2;i++) {
						hand.turn[i/2][i%2]=charToInt(hold[i]);
					}
					hand.stage=2;
					hand.numBets=0;
					hand.playerToAct=1;
					hand.actionsLeft=hand.numActivePlayers;
				//river command - for inputing the river card
				} else if (command=="river") {
					cin>>hold[0]>>hold[1];
					for(int i=0;i<2;i++) {
						hand.river[i/2][i%2]=charToInt(hold[i]);
					}
					hand.stage=3;
					hand.numBets=0;
					hand.playerToAct=1;
					hand.actionsLeft=hand.numActivePlayers;
				//fold command - Jim's opponent folds
				} else if (command=="fold") {
					hand.activePlayers[hand.playerToAct]=0;
					hand.numActivePlayers--;
					hand.playerToAct++;
					hand.actionsLeft--;
				//check command - Jim's opponent checks
				} else if (command=="check") {
					if(hand.stage==0) {
						float equityU=convertedEquity(hand.numActivePlayers, .6f);
						updateRange(0,equityU,true,&hand.ranges,deck,hand);
					}
					float equityL=convertedEquity(hand.numActivePlayers, .65f);
					float equityU=convertedEquity(hand.numActivePlayers, .8f);
					updateRange(equityL,equityU,false,&hand.ranges,deck,hand);
					hand.playerToAct++;
					hand.actionsLeft--;
				//call command - Jim's opponent calls
				} else if (command=="call") {
					hand.potSize+=hand.callSizes[hand.playerToAct];
					if(hand.stage==0 && hand.numBets==0) {
						float equityL=convertedEquity(hand.numActivePlayers, .4f);
						float equityU=convertedEquity(hand.numActivePlayers, .6f);
						updateRange(equityL,equityU,true,&hand.ranges,deck,hand);
					}
					float equity=(1.0*hand.callSizes[hand.playerToAct])/(1.0*hand.potSize);
					equity=convertedEquity(hand.numActivePlayers,equity);
					updateRange(equity,1,true,&hand.ranges,deck,hand);
					hand.callSizes[hand.playerToAct]=0;
					hand.playerToAct++;
					hand.actionsLeft--;
				//raise command - Jim's opponent raises by a given value
				} else if (command=="raise") {
					hand.numBets++;
					hand.potSize+=hand.callSizes[hand.playerToAct];
					int pre=hand.callSizes[hand.playerToAct];
					cin>>hand.callSizes[hand.playerToAct];
					if(hand.callSizes[hand.playerToAct]>myStack) {
						hand.callSizes[hand.playerToAct]=myStack;
					}
					hand.potSize+=hand.callSizes[hand.playerToAct];
					for(int i=0;i<9;i++) {
						if(i!=hand.playerToAct) {
							hand.callSizes[i]+=hand.callSizes[hand.playerToAct];
						}
					}
					// float equity=(1.0*pre+callSize)/(1.0*potSize);
					// updateRange(equity,&myRange,&oppRange,stage,myCards,deck,flop,turn,river);
					if(hand.stage==0) {
						float equity=convertedEquity(hand.numActivePlayers, .5f);
						updateRange(equity,1,true,&hand.ranges,deck,hand);
					}
					float e=(1.0*hand.callSizes[hand.playerToAct]+pre)/(1.0*hand.potSize+hand.callSizes[hand.playerToAct]);
					e=convertedEquity(hand.numActivePlayers,e);
					float foldEq=foldEquity(e,&hand.ranges,deck,hand);
					float equity=foldEq+(1-foldEq)*(e);
					for(int i=1;i<hand.numBets;i++) {
						equity+=(1-equity)/2;
					}
					updateRange(equity,1,true,&hand.ranges,deck,hand);
					hand.callSizes[hand.playerToAct]=0;
					hand.playerToAct++;
					hand.actionsLeft=hand.numActivePlayers-1;
				//win command - Jim wins the pot
				} else if (command=="straddle") {
					int straddle=0;
					cin>>straddle;
					hand.potSize+=straddle;
					for(int i=0;i<9;i++) {
						if(i!=hand.playerToAct) {
							hand.callSizes[i]+=straddle-big;
						}
					}
					hand.callSizes[hand.playerToAct]=0;
					hand.playerToAct++;
					hand.actionsLeft=hand.numActivePlayers;
				} else if (command=="win") {
					myStack+=hand.potSize;
					hand.potSize=0;
					hand.callSizes[hand.playerToAct]=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//lose command - Jim loses the pot
				} else if (command=="lose") {
					hand.potSize=0;
					hand.callSizes[hand.playerToAct]=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//act command - tells Jim to take his turn and perform action
				} else if (command=="act") {
					action(deck,hand,&myStack,big);
				//reset command - Resets stack sizes in case of user mistake
				} else if (command=="reset") {
					cin>>myStack;
					hand.potSize=0;
					hand.callSizes[hand.playerToAct]=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//blinds command - sets new blinds
				} else if (command=="blinds") {
					cin>>small>>big;
					hand.potSize=0;
					hand.callSizes[hand.playerToAct]=0;
					hand.aggressor=false;
					hand.stage=0;
					handInProgress=false;
				//range command - Jim displays the range of hands he puts his opponent on
				} else if (command=="range") {
					for(int i=0;i<1326;i++) {
						if(hand.ranges[hand.playerToAct][i][0]!=0) {
							cout<<hand.ranges[hand.playerToAct][i][0]<<" "<<hand.ranges[hand.playerToAct][i][1]<<"   "<<hand.ranges[hand.playerToAct][i][2]<<" "<<hand.ranges[hand.playerToAct][i][3]<<endl;
						}
					}
				//incase user inputs invalid command:
				} else if (command=="players") {
					cin>>numPlayers;
					hand.numPlayers=numPlayers;
				} else {
					validCommand=false;
					cout<<"Invalid command... \n";
				}

			}
		}
	}

}

void updateRange(float eL, float eU, bool keepInside, int (*ranges)[9][1326][4], int deck[52][2], Hand& hand) {
	int tempRange[1326][4];
	float equityL = 100*eL;
	float equityU = 100*eU;
	//cout<<"eq:"<<equity<<endl;
	for(int i=0;i<1326;i++) {
		tempRange[i][0]=(*ranges)[hand.playerToAct][i][0];
		tempRange[i][1]=(*ranges)[hand.playerToAct][i][1];
		tempRange[i][2]=(*ranges)[hand.playerToAct][i][2];
		tempRange[i][3]=(*ranges)[hand.playerToAct][i][3];
	} 
	int cards[2][2];
	for(int i=0;i<1326;i++) {
		if((*ranges)[hand.playerToAct][i][0]!=0) {
			cards[0][0]=(*ranges)[hand.playerToAct][i][0];
			cards[0][1]=(*ranges)[hand.playerToAct][i][1];
			cards[1][0]=(*ranges)[hand.playerToAct][i][2];
			cards[1][1]=(*ranges)[hand.playerToAct][i][3];
			bool dupe = false;
			if(hand.stage>=0) {
				for(int j=0;j<2;j++) {
					if(cards[j][0]==hand.myCards[0][0] && cards[j][1]==hand.myCards[0][1]) {
						dupe=true;
					}
				}
			} if(hand.stage>=1) {
				for(int j=0;j<2;j++) {
					if(cards[j][0]==hand.flop[0][0] && cards[j][1]==hand.flop[0][1]) {
						dupe=true;
					} else if (cards[j][0]==hand.flop[1][0] && cards[j][1]==hand.flop[1][1]) {
						dupe=true;
					} else if (cards[j][0]==hand.flop[2][0] && cards[j][1]==hand.flop[2][1]) {
						dupe=true;
					}
				}
			} else if (hand.stage>=2) {
				for(int j=0;j<2;j++) {
					if(cards[j][0]==hand.turn[0][0] && cards[j][1]==hand.turn[0][1]) {
						dupe=true;
					}
				}
			} else if (hand.stage>=3) {
				for(int j=0;j<2;j++) {
					if(cards[j][0]==hand.river[0][0] && cards[j][1]==hand.river[0][1]) {
						dupe=true;
					}
				}
			}
			if(dupe) {
				for(int j=0;j<4;j++) {
					(*ranges)[hand.playerToAct][i][j]=0;
				}
			} else {
				//loop for each hand in range1, calculate winrate against range2, if winrate is not between lower and upper bounds, remove hand from range1 (set to 0)
				int winRate=simHands(cards,52,deck,100,hand,ranges,&tempRange,false);
				if(keepInside) {
					if((winRate<equityL || winRate>equityU) && !potential(cards,hand)) {
						//cout<<winRate<<"<"<<equity<<"Eliminated "<<(*range1)[i][0]<<(*range1)[i][1]<<" "<<(*range1)[i][2]<<(*range1)[i][3]<<endl;
						for(int j=0;j<4;j++) {
							(*ranges)[hand.playerToAct][i][j]=0;
						}
					} 
				} else {
					if(winRate>equityL && winRate<equityU) {
						//cout<<winRate<<"<"<<equity<<"Eliminated "<<(*range1)[i][0]<<(*range1)[i][1]<<" "<<(*range1)[i][2]<<(*range1)[i][3]<<endl;
						for(int j=0;j<4;j++) {
							(*ranges)[hand.playerToAct][i][j]=0;
						}
					} 
				}
			}
		}
	}
}

bool potential(int cards[2][2],Hand& hand) {
	if(hand.stage==0)
		return false;
	int suits[4]={0};
	int sorted[5][2]={{cards[0][0],cards[0][1]},{cards[1][0],cards[1][1]},{hand.flop[0][0],hand.flop[0][1]},{hand.flop[1][0],hand.flop[1][1]},{hand.flop[2][0],hand.flop[2][1]}};
	
	suits[cards[0][1]-1]++;
	suits[cards[0][1]-1]++;
	suits[hand.flop[0][1]-1]++;
	suits[hand.flop[1][1]-1]++;
	suits[hand.flop[2][1]-1]++;
	//counts number of each suit and if hand has 4 of one suit after the flop, return true
	for(int i=0;i<4;i++) {
		if(suits[i]==4) {
			return true;
		}
	}
	//sort the hand to look for straight potential
	int key1,key2,j;
	for(int i=1;i<5;i++) {
		key1=sorted[i][0];
		key2=sorted[i][1];
		j=i-1;
		while(j>=0 && sorted[j][0]<key1) {
			sorted[j+1][0]=sorted[j][0];
			sorted[j+1][1]=sorted[j][1];
			j--;
		}
		sorted[j+1][0]=key1;
		sorted[j+1][1]=key2;
	}
	//return true if hand has open ended straight potential
	if(sorted[4][0]-sorted[1][0]==3 && sorted[4][0]!=sorted[1][0] && sorted[4][0]!=sorted[2][0] && sorted[4][0]!=sorted[3][0]) {
		return true;
	} else if(sorted[3][0]-sorted[0][0]==3 && sorted[0][0]!=sorted[1][0] && sorted[0][0]!=sorted[2][0] && sorted[0][0]!=sorted[3][0]) {
		return true;
	} //missing case where 4 cards in a row and 5th is a mid-pair
	return false;
}

float foldEquity(float e, int (*ranges)[9][1326][4], int deck[52][2], Hand& hand) {
	int tempRange[1326][4];
	float equity = 100*e;
	//cout<<"eq:"<<equity<<endl;
	for(int i=0;i<1326;i++) {
		tempRange[i][0]=(*ranges)[hand.playerToAct][i][0];
		tempRange[i][1]=(*ranges)[hand.playerToAct][i][1];
		tempRange[i][2]=(*ranges)[hand.playerToAct][i][2];
		tempRange[i][3]=(*ranges)[hand.playerToAct][i][3];
	} 
	int cards[2][2];
	float folds=0;
	float trials;
	for(int i=0;i<1326;i++) {
		if((*ranges)[hand.playerToAct][i][0]!=0) {
			cards[0][0]=(*ranges)[hand.playerToAct][i][0];
			cards[0][1]=(*ranges)[hand.playerToAct][i][1];
			cards[1][0]=(*ranges)[hand.playerToAct][i][2];
			cards[1][1]=(*ranges)[hand.playerToAct][i][3];
			//loop for each hand in range1, tests winrate against range2 and counts the number of hands that are <equity
			int winRate=simHands(cards,52,deck,100,hand,ranges,&tempRange,false);
			if(winRate<equity) {
				folds++;
			} 
			trials++;
		}
	}
	//returns percentage of hands folded
	return folds/trials;
}

void resetRange(int (*range)[1326][4],int deck[52][2]) {
	int n=52,r=2;
	vector<bool> v(n);
	int j=0;
	int k=0;
	//refills the given range with every possible hand in the deck
    fill(v.end() - r, v.end(), true);
    do {
        for (int i=0;i<n;i++) {
            if (v[i]) {
            	(*range)[j][k]=deck[i][0];
            	(*range)[j][k+1]=deck[i][1];
            	k+=2;
            }
        }
        j++;
        k=0;
    } while (next_permutation(v.begin(), v.end()));
}

int charToInt(char c) {
	switch(c) {
		case 'A':
		case 'a':
			return 14;
			break;
		case 'K':
		case 'k':
			return 13;
			break;
		case 'Q':
		case 'q':
			return 12;
			break;
		case 'J':
		case 'j':
			return 11;
			break;
		case 'T':
		case 't':
			return 10;
			break;
		case 'C':
		case 'c':
			return 1;
			break;
		case 'D':
		case 'd':
			return 2;
			break;
		case 'H':
		case 'h':
			return 3;
			break;
		case 'S':
		case 's':
			return 4;
			break;
		default:
			return c-'0';
			break;
	}
}

//converts a given heads-up-based equity to an equity based on the number of players
//y = 1.6323x^-0.669 R² = 0.9972
float convertedEquity(int numPlayers, float equity) {
	return (2.0 / numPlayers) * equity;
}

	/*
	Straight Flush		90,000,000,000
	Four of a Kind		8
	Full House			7
	Flush				6
	Straight			5
	Three of a Kind		4
	Two Pair			3
	Pair				2
	High Card 			1
	*/

float strengthOfHand(int cards[5][2]) {

	int key1,key2,j;
	for(int i=1;i<5;i++) {
		key1=cards[i][0];
		key2=cards[i][1];
		j=i-1;
		while(j>=0 && cards[j][0]<key1) {
			cards[j+1][0]=cards[j][0];
			cards[j+1][1]=cards[j][1];
			j--;
		}
		cards[j+1][0]=key1;
		cards[j+1][1]=key2;
	}

	//Four of a Kind
	if (cards[0][0]==cards[1][0] && cards[0][0]==cards[2][0] && cards[0][0]==cards[3][0]) {
		return 8000 + cards[4][0];
	} else if (cards[4][0]==cards[1][0] && cards[4][0]==cards[2][0] && cards[4][0]==cards[3][0]) {
		return 8000 + cards[0][0];
	}

	//Full House
	if (cards[0][0]==cards[1][0] && cards[0][0]==cards[2][0] && cards[3][0]==cards[4][0]) {
		return 7000 + cards[0][0] * 10 + cards[4][0] * .1;
	} else if (cards[0][0]==cards[1][0] && cards[2][0]==cards[3][0] && cards[2][0]==cards[4][0]) {
		return 7000 + cards[4][0] * 10 + cards[0][0] * .1;
	}

	bool flush=false;
	//Flush
	if (cards[0][1]==cards[1][1] && cards[0][1]==cards[2][1] && cards[0][1]==cards[3][1] && cards[0][1]==cards[4][1]) {
		flush=true;
	}

	//Straight and Straight Flush
	if (cards[0][0]-cards[4][0]==4 && cards[0][0]!=cards[1][0] && cards[1][0]!=cards[2][0] && cards[2][0]!=cards[3][0] && cards[3][0]!=cards[4][0]) {
		if(flush) {
			return 9000 + cards[0][0];
		} else {
			return 5000 + cards[0][0];
		}
	} else if (cards[0][0]==14 && cards[1][0]==5 && cards[2][0]==3 && cards[3][0]==3 && cards[4][0]==2) {
		if(flush) {
			return 9000 + cards[1][0];
		} else {
			return 5000 + cards[1][0];
		}
	} else if (flush) {
		return 6000 + cards[0][0] * 10 + cards[1][0] * .1 + cards[2][0] * .001 + cards[3][0] * .00001 + cards[4][0] * .0000001;
	}

	//Three of a Kind
	if (cards[0][0]==cards[1][0] && cards[0][0]==cards[2][0]) {
		return 4000 + cards[0][0]*10 + cards[3][0]*.1 + cards[4][0]*.001;
	} else if (cards[1][0]==cards[2][0] && cards[1][0]==cards[3][0]) {
		return 4000 + cards[1][0]*10 + cards[0][0]*.1 + cards[4][0]*.001;
	} else if (cards[2][0]==cards[3][0] && cards[2][0]==cards[4][0]) {
		return 4000 + cards[2][0]*10 + cards[0][0]*.1 + cards[1][0]*.001;
	}

	//Two Pair
	if (cards[0][0]==cards[1][0] && cards[2][0]==cards[3][0]) {
		return 3000 + cards[0][0]*10 + cards[2][0]*.1 + cards[4][0]*.001;
	} else if (cards[0][0]==cards[1][0] && cards[3][0]==cards[4][0]) {
		return 3000 + cards[0][0]*10 + cards[3][0]*.1 + cards[2][0]*.001;
	} else if (cards[1][0]==cards[2][0] && cards[3][0]==cards[4][0]) {
		return 3000 + cards[1][0]*10 + cards[3][0]*.1 + cards[0][0]*.001;
	}

	//Pair
	if (cards[0][0]==cards[1][0]) {
		return 2000 + cards[0][0]*10 + cards[2][0]*.1 + cards[3][0]*.001 + cards[4][0]*.00001;
	} else if (cards[1][0]==cards[2][0]) {
		return 2000 + cards[1][0]*10 + cards[0][0]*.1 + cards[3][0]*.001 + cards[4][0]*.00001;
	} else if (cards[2][0]==cards[3][0]) {
		return 2000 + cards[2][0]*10 + cards[0][0]*.1 + cards[1][0]*.001 + cards[4][0]*.00001;
	} else if (cards[3][0]==cards[4][0]) {
		return 2000 + cards[3][0]*10 + cards[0][0]*.1 + cards[1][0]*.001 + cards[2][0]*.00001;
	}

	//High Card
	return 1000 + cards[0][0]*10 + cards[1][0]*.1 + cards[2][0]*.001 + cards[3][0]*.00001 + cards[4][0]*.0000001;
}

void action(int deck[52][2],Hand& hand,int *myStack,int big) {
	int preflopDeck[50][2];
	int flopDeck[47][2];
	int turnDeck[46][2];
	int riverDeck[45][2];

	if (hand.stage==0) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1]))) {
				preflopDeck[j][0]=deck[i][0];
				preflopDeck[j][1]=deck[i][1];
				j++;
			}
		}
		
		float winRate = simHands(hand.myCards,50,preflopDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],false);
		float rangeWinRate = simHands(hand.myCards,50,preflopDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],true);
		cout<<winRate<<","<<rangeWinRate<<endl;

		if(*myStack / big < 15) {
			if(winRate > convertedEquity(hand.numActivePlayers, 50 + *myStack / big)) {
				Raise(myStack,hand,*myStack,deck);
			} else if (hand.callSizes[hand.position] == 0) {
				Check(myStack,hand,deck);
			} else {
				Fold(myStack,hand);
			}
		} else if(hand.callSizes[hand.position] == 0) {
			if(winRate > convertedEquity(hand.numActivePlayers, 60)) {
				Raise(myStack,hand,2*big,deck);
			} else {
				Check(myStack,hand,deck);
			}
		} else if (hand.numBets==0) {
			if(winRate > convertedEquity(hand.numActivePlayers, 40+hand.numActivePlayers*2.5)) {
				Raise(myStack,hand,hand.potSize,deck);
			} else {
				Fold(myStack,hand);
			}
		} else {
			if(winRate > convertedEquity(hand.numActivePlayers, 65)) {
				Raise(myStack,hand,hand.potSize,deck);
			} else if (winRate > 100.0 * hand.callSizes[hand.position] / (1.0*hand.potSize+hand.callSizes[hand.position]) - 5 || winRate > convertedEquity(hand.numActivePlayers, 60)) {
				Call(myStack,hand,deck);
			} else {
				Fold(myStack,hand);
			}
		}

	} else if(hand.stage==1) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1])) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1])) {
				flopDeck[j][0]=deck[i][0];
				flopDeck[j][1]=deck[i][1];
				j++;
			}
		}

		float winRate = simHands(hand.myCards,47,flopDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],false);
		float rangeWinRate = simHands(hand.myCards,47,flopDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],true);
		cout<<winRate<<","<<rangeWinRate<<endl;

		if(hand.callSizes[hand.position] == 0) {
			if(rangeWinRate > convertedEquity(hand.numActivePlayers, 65)) {
				Raise(myStack,hand,(rangeWinRate/100)*hand.potSize,deck);
			} else if (winRate > convertedEquity(hand.numActivePlayers, 50)) {
				Raise(myStack,hand,(winRate/100)*hand.potSize,deck);
			} else {
				Check(myStack,hand,deck);
			}
		} else {
			if(winRate > convertedEquity(hand.numActivePlayers, 70) || rangeWinRate > convertedEquity(hand.numActivePlayers, 65)) {
				Raise(myStack,hand,(rangeWinRate/100)*hand.potSize,deck);
			} else if((winRate > 100.0 * hand.callSizes[hand.position] / (1.0*hand.potSize+hand.callSizes[hand.position]) && !(winRate < convertedEquity(hand.numActivePlayers, 35))) || potential(hand.myCards,hand)) {
				Call(myStack,hand,deck);
			} else {
				Fold(myStack,hand);
			}
		}
		
	} else if (hand.stage==2) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1])) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.turn[1][0] && deck[i][1]==hand.turn[1][1])) {
				turnDeck[j][0]=deck[i][0];
				turnDeck[j][1]=deck[i][1];
				j++;
			}
		}

		float winRate = simHands(hand.myCards,46,turnDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],false);
		float rangeWinRate = simHands(hand.myCards,46,turnDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],true);
		cout<<winRate<<","<<rangeWinRate<<endl;

		if(hand.callSizes[hand.position] == 0) {
			if(rangeWinRate > convertedEquity(hand.numActivePlayers, 60)) {
				Raise(myStack,hand,(rangeWinRate/100)*hand.potSize,deck);
			} else if (winRate > convertedEquity(hand.numActivePlayers, 60) && rangeWinRate < convertedEquity(hand.numActivePlayers, 55)) {
				//check to induce, maybe check raise
				Check(myStack,hand,deck);
			} else if (winRate > convertedEquity(hand.numActivePlayers, 50)) {
				Raise(myStack,hand,(winRate/100)*hand.potSize,deck);
			} else {
				Check(myStack,hand,deck);
			}
		} else {
			if(winRate > convertedEquity(hand.numActivePlayers, 60) && rangeWinRate < convertedEquity(hand.numActivePlayers, 55)) {
				Raise(myStack,hand,(winRate/100)*hand.potSize,deck);
			} else if(winRate > convertedEquity(hand.numActivePlayers, 70) || rangeWinRate > convertedEquity(hand.numActivePlayers, 60)) {
				Raise(myStack,hand,(rangeWinRate/100)*hand.potSize,deck);
			} else if((winRate > 100.0 * hand.callSizes[hand.position] / (1.0*hand.potSize+hand.callSizes[hand.position]) && !(winRate < convertedEquity(hand.numActivePlayers, 35)))) {
				Call(myStack,hand,deck);
			} else {
				Fold(myStack,hand);
			}
		}

	} else if (hand.stage==3) {

		int j=0;
		for(int i=0;i<52;i++) {
			if(!((deck[i][0]==hand.myCards[0][0] && deck[i][1]==hand.myCards[0][1]) || (deck[i][0]==hand.myCards[1][0] && deck[i][1]==hand.myCards[1][1])) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.flop[1][0] && deck[i][1]==hand.flop[1][1]) || (deck[i][0]==hand.turn[1][0] && deck[i][1]==hand.turn[1][1]) || (deck[i][0]==hand.river[1][0] && deck[i][1]==hand.river[1][1])) {
				riverDeck[j][0]=deck[i][0];
				riverDeck[j][1]=deck[i][1];
				j++;
			}
		}

		float winRate = simHands(hand.myCards,45,riverDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],false);
		float rangeWinRate = simHands(hand.myCards,45,riverDeck,10000,hand,&hand.ranges,&hand.ranges[hand.position],true);
		cout<<winRate<<","<<rangeWinRate<<endl;

		if(hand.callSizes[hand.position] == 0) {
			if(rangeWinRate > convertedEquity(hand.numActivePlayers, 55)) {
				Raise(myStack,hand,(rangeWinRate/100)*hand.potSize,deck);
			} else if (winRate > convertedEquity(hand.numActivePlayers, 60) && rangeWinRate < convertedEquity(hand.numActivePlayers, 50)) {
				//check to induce, maybe check raise
				Check(myStack,hand,deck);
			} else if (winRate > convertedEquity(hand.numActivePlayers, 50)) {
				Raise(myStack,hand,(winRate/100)*hand.potSize,deck);
			} else {
				Check(myStack,hand,deck);
			}
		} else {
			if(winRate > convertedEquity(hand.numActivePlayers, 60) && rangeWinRate < convertedEquity(hand.numActivePlayers, 55)) {
				Raise(myStack,hand,(winRate/100)*hand.potSize,deck);
			} else if(winRate > convertedEquity(hand.numActivePlayers, 70) || rangeWinRate > convertedEquity(hand.numActivePlayers, 60)) {
				Raise(myStack,hand,(rangeWinRate/100)*hand.potSize,deck);
			} else if((winRate > 100 * hand.callSizes[hand.position] / (hand.potSize+hand.callSizes[hand.position]) && !(winRate < convertedEquity(hand.numActivePlayers, 35)))) {
				Call(myStack,hand,deck);
			} else {
				Fold(myStack,hand);
			}
		}

	}

}

float simHands(int mc[2][2],int n,int inDeck[n][2],int trials,Hand& hand,int (*ranges)[9][1326][4],int (*myRange)[1326][4],bool range) {
	int deck[52][2];
	for(int i=0;i<n;i++) {
		deck[i][0]=inDeck[i][0];
		deck[i][1]=inDeck[i][1];
	}
	int board[5][2]={{0,0},{0,0},{0,0},{0,0},{0,0}};
	float wins=0;
	int myCards[2][2];
	myCards[0][0]=mc[0][0];
	myCards[0][1]=mc[0][1];
	myCards[1][0]=mc[1][0];
	myCards[1][1]=mc[1][1];
	for(int b=0;b<trials;b++) {
		int counter=0,counter1=0;
		if(range) {
			int rnd;
			do {
			    random_device rd;	
			    mt19937 gen(rd()); 
				uniform_int_distribution<int> distribution(0,1325);
			    rnd=distribution(gen);
				counter1++;
				if(counter1>5000) {
					resetRange(myRange,deck);
					counter1=0;
				}
			} while((*myRange)[rnd][0]==0);
			myCards[0][0]=(*myRange)[rnd][0];
			myCards[0][1]=(*myRange)[rnd][1];
			myCards[1][0]=(*myRange)[rnd][2];
			myCards[1][1]=(*myRange)[rnd][3];
		}

		int oppCards[hand.numActivePlayers-1][2][2];
		int rand;
		int kk=0;
		for(int j=0;j<9;j++) {
			if(!(hand.activePlayers[j]==0 || j==hand.position)) {
				do {
				    random_device rd;  
				    mt19937 gen(rd()); 
					uniform_int_distribution<int> distribution(0,1325);
					rand=distribution(gen);
					counter++;
					if(counter>5000) {
						resetRange(&((*ranges)[j]),deck);
						counter=0;
					}
				} while((*ranges)[j][rand][0]==0);
				oppCards[kk][0][0]=(*ranges)[j][rand][0];
				oppCards[kk][0][1]=(*ranges)[j][rand][1];
				oppCards[kk][1][0]=(*ranges)[j][rand][2];
				oppCards[kk][1][1]=(*ranges)[j][rand][3];
				kk++;
			}
		}

		random_device rd;
		mt19937 g(rd());
		shuffle(begin(deck),end(deck),g);
		for(int i=0;i<hand.numActivePlayers-1;i++) {
			while((deck[0][0]==oppCards[i][0][0] && deck[0][1]==oppCards[i][0][1]) || (deck[1][0]==oppCards[i][1][0] && deck[1][1]==oppCards[i][1][1])) {
				random_device rd;
				mt19937 g(rd());
				shuffle(begin(deck),end(deck),g);
			}
		}
		for(int i=0;i<5;i++) {
			board[i][0]=deck[i][0];
			board[i][1]=deck[i][1];
		}
		if(hand.stage>=1) {
			for(int i=0;i<3;i++) {
				board[i][0]=hand.flop[i][0];
				board[i][1]=hand.flop[i][1];
			}
		}
		if(hand.stage>=2) {
			board[3][0]=hand.turn[0][0];
			board[3][1]=hand.turn[0][1];
		}
		if(hand.stage>=3) {
			board[4][0]=hand.river[0][0];
			board[4][1]=hand.river[0][1];
		}

		// cout<<"\n"<<myCards[0][0]<<"."<<myCards[0][1]<<" "<<myCards[1][0]<<"."<<myCards[1][1]<<" vs "<<oppCards[1][0][0]<<"."<<oppCards[1][0][1]<<" "<<oppCards[1][1][0]<<"."<<oppCards[1][1][1]<<"\n";
		// cout<<board[0][0]<<"."<<board[0][1]<<" "<<board[1][0]<<"."<<board[1][1]<<" "<<board[2][0]<<"."<<board[2][1]<<" "<<board[3][0]<<"."<<board[3][1]<<" "<<board[4][0]<<"."<<board[4][1]<<"\n";

		int n=7,r=5;
		vector<bool> v(n);
	    fill(v.end() - r, v.end(), true);
	    int k=0;
	    int l=0;
	    int myCombinations[21][5][2];
	    int oppCombinations[hand.numActivePlayers-1][21][5][2];
	    do {
	        for (int i=0;i<n;i++) {
	            if (v[i]) {
	            	if (i<5) {
	            		myCombinations[k][l][0]=board[i][0];
	            		myCombinations[k][l][1]=board[i][1];
	            		for(int ii=0;ii<hand.numActivePlayers-1;ii++) {
		            		oppCombinations[ii][k][l][0]=board[i][0];
		            		oppCombinations[ii][k][l][1]=board[i][1];
	            		}
	            	} else {
	            		myCombinations[k][l][0]=myCards[i-5][0];
	            		myCombinations[k][l][1]=myCards[i-5][1];
	            		for(int ii=0;ii<hand.numActivePlayers-1;ii++) {
		            		oppCombinations[ii][k][l][0]=oppCards[ii][i-5][0];
		            		oppCombinations[ii][k][l][1]=oppCards[ii][i-5][1];
	            		}
	            	}
	            	l++;
	            }
	        }
	        l=0;
	        k++;
	    } while (next_permutation(v.begin(), v.end()));

	    float myMaxStrength;
	    float oppMaxStrength;
	    myMaxStrength=0;
	    oppMaxStrength=0;
	    for(int i=0;i<21;i++) {
	    	float myTemp=strengthOfHand(myCombinations[i]);
	    	if(myTemp>myMaxStrength) {
	    		myMaxStrength=myTemp;
	    	}
	    	for(int j=0;j<hand.numActivePlayers-1;j++) {
		    	float oppTemp=strengthOfHand(oppCombinations[j][i]);
		    	//cout<<oppCombinations[j][i][0][0]<<endl;
		    	if(oppTemp>oppMaxStrength) {
		    		oppMaxStrength=oppTemp;
		    	}
	    	}
	    }
	    //cout<<myMaxStrength<<" "<<oppMaxStrength<<"\n";
	    if(myMaxStrength>=oppMaxStrength) {
	    	wins++;
	    }/*
	    	cout<<"WIN:"<<endl;
		    for(int i=0;i<4;i++) {
				cout<<oppCards[i/2][i%2]<<" ";
			}
			cout<<" vs ";
			for(int i=0;i<4;i++) {
				cout<<myCards[i/2][i%2]<<" ";
			}
			cout<<endl;
			for(int i=0;i<10;i++) {
				cout<<board[i/2][i%2]<<" ";
			}
			cout<<endl;
	    
	    if(oppMaxStrength>=myMaxStrength&&!range) {
	    	cout<<"LOSS:"<<endl;
	    	for(int i=0;i<4;i++) {
	    		cout<<oppCards[i/2][i%2]<<" ";
	    	}
	    	cout<<" vs ";
	    	for(int i=0;i<4;i++) {
	    		cout<<myCards[i/2][i%2]<<" ";
	    	}
	    	cout<<endl;
	    	for(int i=0;i<10;i++) {
	    		cout<<board[i/2][i%2]<<" ";
	    	}
	    	cout<<endl;
	    } else if (myMaxStrength==oppMaxStrength) {
	    	wins+=.5;
	    }*/
	}
	float winRate = 100.0 * wins / trials;
	//if(!range) cout<<winRate<<endl;
	return winRate;
}

void Fold(int *myStack,Hand& hand) {
	cout<<"Fold.\n";
	hand.potSize=0;
	hand.callSizes[hand.position]=0;
	hand.aggressor=false;
	hand.activePlayers[hand.position]=0;
	hand.playerToAct++;
	hand.actionsLeft--;
}

void Check(int *myStack,Hand& hand,int deck[52][2]) {
	cout<<"Check.\n";
	hand.aggressor=false;
	if(hand.stage==0) {
		float equityU=convertedEquity(hand.numActivePlayers, .6f);
		updateRange(0,equityU,true,&hand.ranges,deck,hand);
	}
	float equityL=convertedEquity(hand.numActivePlayers, .65f);
	float equityU=convertedEquity(hand.numActivePlayers, .8f);
	updateRange(equityL,equityU,false,&hand.ranges,deck,hand);
	hand.playerToAct++;
	hand.actionsLeft--;
}

void Call(int *myStack,Hand& hand,int deck[52][2]) {
	cout<<"Call.\n";
	hand.potSize+=hand.callSizes[hand.position];
	*myStack-=hand.callSizes[hand.position];
	if(hand.stage==0) {
		float equityL=convertedEquity(hand.numActivePlayers, .4f);
		float equityU=convertedEquity(hand.numActivePlayers, .6f);
		updateRange(equityL,equityU,true,&hand.ranges,deck,hand);
	}
	float equity=(1.0*hand.callSizes[hand.position])/(1.0*hand.potSize);
	equity=convertedEquity(hand.numActivePlayers,equity);
	hand.callSizes[hand.position]=0;
	hand.aggressor=false;
	updateRange(equity,100,true,&hand.ranges,deck,hand);
	hand.playerToAct++;
	hand.actionsLeft--;
}

void Raise(int *myStack,Hand& hand,int value,int deck[52][2]) {
	hand.aggressor=true;
	value=value+(hand.smallBlind-value%hand.smallBlind);
	if(value>*myStack-hand.callSizes[hand.position]-value) {
		value=*myStack-hand.callSizes[hand.position];
	}
	if(value==0) {
		Call(myStack,hand,deck);
		return;
	}
	cout<<"Raise by "<<value<<endl;

	hand.potSize+=hand.callSizes[hand.position]+value;
	*myStack-=hand.callSizes[hand.position]+value;
	int pre=hand.callSizes[hand.position];
	for(int i=0;i<9;i++) {
		if(i!=hand.playerToAct) {
			hand.callSizes[i]+=value;
		}
	}

	if(hand.stage==0 && hand.numBets==0) {
		float equity=convertedEquity(hand.numActivePlayers, .5f);
		updateRange(equity,100,true,&hand.ranges,deck,hand);
	}
	float e=(1.0*value+hand.callSizes[hand.position])/(1.0*hand.potSize+value);
	e=convertedEquity(hand.numActivePlayers,e);
	float foldEq=foldEquity(e,&hand.ranges,deck,hand);
	float equity=foldEq+(1-foldEq)*(e);
	updateRange(equity,100,true,&hand.ranges,deck,hand);
	hand.callSizes[hand.position]=0;
	hand.playerToAct++;
	hand.actionsLeft=hand.numActivePlayers-1;
}







 




















