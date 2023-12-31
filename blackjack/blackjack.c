#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define A 14
#define K 13
#define Q 12
#define J 11

//defines players
typedef struct player
{
    char name[25];
    int score;
    int bank;
    int cards[12];
    int suit[12];
    int bet;
    int card_count;
    //Pointer to next struct for linked list
    struct player *next;
}
player;

//Assigns players at the start of the game
player *assign_player(int player_count, FILE *player_data);
//Returns linked list back from last player to first
player *start_of_list(player *p, int player_count);

//Calculates player's scores
int do_score(player *p);

//Prints each player's score
void print_scores(player *p, int player_count);
//Free's allocated memory for player struct
void free_players(player *p);
//Show's play's current hands
void see_hand(player *p);
//Randomly selects cards for player
void random_card(int card_pos[2]);
//Show's player's hand
void print_card(int cards_pos[], int current_card);

int main(void)
{
    const int MIN_BET = 20;
    //Matrix as deck of cards
    int cards[4][13] = {{2,3,4,5,6,7,8,9,10,J,Q,K,A}, 
                        {2,3,4,5,6,7,8,9,10,J,Q,K,A},
                        {2,3,4,5,6,7,8,9,10,J,Q,K,A},
                        {2,3,4,5,6,7,8,9,10,J,Q,K,A}};
    int player_count;
    player *dealer = (player*)malloc(sizeof(player));
    //runs if error occurs
    if(dealer == NULL)
    {
       printf("Error generating Dealer.\nSystem shutting down...\n");
        return 1;
    }
    printf("*******************************************\n");
    printf("*********  Welcome to Blackjack!  *********\n");
    printf("*******************************************\n\n");
    //Runs while player_count isn't between 1 and 7
    do
    {
        printf("Amount of players must be between 1 and 7\n");
        printf("Enter amount of players: ");
        scanf("%i", &player_count);
    } while (player_count < 1 || player_count > 7);
    printf("\nKeep player names no larger than 25 characters\n");
    printf("Player names may also not include any spaces\n\n");
    //Open's file for reading player data
    FILE *player_data;
    player_data = fopen("players.txt", "r");
    if(player_data == NULL)
    {
        //If file does not exist, it is created
        player_data = fopen("players.txt", "w");
        fclose(player_data);
        //Reopens for reading
        player_data = fopen("players.txt", "r");
        if(player_data == NULL)
        {
            printf("An Error Has Occured\nShutting System Down...\n");
            return 1;
        }
    }
    //assign each players name and score into struct p
    player *p = assign_player(player_count, player_data);
    //runs if error occurs
    if(p == NULL)
    {
        printf("Error generating players.\nSystem shutting down...\n");
        return 1;
    }
    fclose(player_data);
    //prints player name and starting score
    printf("\n");
    print_scores(p, player_count);
    printf("\n");
    //temporarily holds p, since p will be temporarily altered
    player *current = p;
    player *prev, *next = NULL;
    //Gets player bets
    for(int i = 0; i < player_count; i++)
    {
        int bet;
        /*
        bank stores player's bank and will subtract the player's bet from it
        If bank - bet is less than zero, the player's bank will be left unchanged and will be asked to input another bet
        */
        int bank;
        do
        {
            bank = current->bank;
            printf("%s place your bet:\n", current->name);
            scanf("%i", &bet);
            bank -= bet;
            //Checks to see if player's bet is less than the minimum bet
            if(bet < MIN_BET)
            {
                printf("The minimum bet is %i, please enter another bet\n", MIN_BET);
                continue;
            }
            else if(bank < 0)
            {
                printf("\nThe amount you entered exceeds your current bank, please enter another bet\n");
                continue;
            }
            current->bet = bet;
            current->bank = bank;
        }
        while(bank < 0 || bet < MIN_BET);
        /*
        next will hold the node pointing to the next player in the linked list
        current->next will equal what prev was, in this case the node will be pointing backwards to reverse the linked list
        prev will equal the current player
        current = next, will move to the next player in the linked list
        */
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
    //return back to start of list p
    p = start_of_list(p, player_count);
    printf("\n");
    current = p;
    prev = NULL;
    next = NULL;
    //assigns starting cards to players
    for(int i = 0; i < player_count; i++)
    {
        printf("%s's cards:\n", current->name);
        for(int j = 0; j < 2; j++)
        {
            //card_poas[0] is the suit of the card, card_pos[1] is the card number
            int card_pos[2];
            do
            {
                //Assigns each player cards
                random_card(card_pos);
                current->cards[j] = cards[card_pos[0]][card_pos[1]];
                current->suit[j] = card_pos[0];
            } 
            //sometimes resulted in numbers outside of the bounds
            //Set bounds in while loop to always be within bounds
            while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
            print_card(card_pos, current->cards[j]);
            if(j == 0)
            {
                printf(", ");
            }
            //removes drawn card from the deck
            cards[card_pos[0]][card_pos[1]] = 0;
        }
        current->card_count = 2;
        //calculates player's score
        current->score = do_score(current);
        printf("\n");
        //moves forward and reverses linked list
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    //setting linked list p, back to start of the list
    p = prev;
    //return back to start of list p
    p = start_of_list(p, player_count);
    if(p == NULL)
    {
        return 1;
    }
    //assigns dealer's cards
    printf("Dealer's cards:\n");
    for(int j = 0; j < 2; j++)
    {
        int card_pos[2];
        do
        {
            //assigns dealer a random card
            random_card(card_pos);
            dealer->cards[j] = cards[card_pos[0]][card_pos[1]];
            dealer->suit[j] = card_pos[0];
        } 
        while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
        //prints dealer's first card(face up)
        if(j == 0)
        {
            print_card(card_pos, dealer->cards[j]);
            printf(", ");
        }
        //second card is not shown(face down)
        else
        {
            printf("?\n");
        }
        //removes selected card from the deck
        cards[card_pos[0]][card_pos[1]] = 0;
    }
    dealer->card_count = 2;
    printf("\n");
    //Determines results of whether player hits, passes, or doubles down
    prev = NULL;
    next = NULL;
    current = p;
    for (int i = 0; i < player_count; i++) 
    {
        char option[7];
        int retry = 1;
        /*
        player will have to stand if their hand is equal to 21
        they will recieve 1.5x their bet
        */
         if(current->score == 21)
            {
                printf("%s, You've already won with a score of 21. You get 1.5x your bet\n", current->name);
                //retry = 0 will skip options for hit, stand, or double down
                retry = 0;
            }
        //runs while retry == 1
        while(retry == 1)
        {
            //shows player's current hand
            printf("%s these are your cards:\n", current->name);
            see_hand(current);
            printf("Please select one of the options below\n");
            printf("(H)it, (S)tand");
            //only provide player to double down if they have the bank to 2x their bet
            if(current->bank >= current->bet)
            {
                printf(", (D)ouble");
            }
            printf("\n");
            scanf("%s", option);
            //runs if player chooses to Pass
            if(strcmp(option, "Stand") == 0 || strcmp(option, "stand") == 0 || strcmp(option, "STAND") == 0 || strcmp(option, "S") == 0 || strcmp(option, "s") == 0)
            {
                retry = 0;
            }
            //runs if player chooses to Double Down
            else if(strcmp(option, "Double") == 0 || strcmp(option, "double") == 0 || strcmp(option, "DOUBLE") == 0 || strcmp(option, "D") == 0 || strcmp(option, "d") == 0)
            {
                //will not run if the player cannot afford to 2x their bet
                if(current->bank < current->bet)
                {
                    printf("\nYou don't have enough in your bank to Double Down\nPlease try again\n\n");
                    sleep(2);
                    continue;
                }
                //calculates player's new bet
                int bet = current->bet;
                int new_bet = bet * 2;
                current->bank -= bet;
                current->bet = new_bet;
                printf("Your new bet is: %i\n", current->bet);
                printf("Your card:\n");
                int card_pos[2];
                do
                {
                    //assigns player another random card
                    random_card(card_pos);
                    current->cards[2] = cards[card_pos[0]][card_pos[1]];
                    current->suit[2] = card_pos[0];
                } 
                while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
                current->card_count++;
                //prints player's new card
                print_card(card_pos, current->cards[2]);
                cards[card_pos[0]][card_pos[1]] = 0;
                printf("\n");
                //shows players current hand
                printf("Your current hand:\n");
                see_hand(current);
                //calculates player's new score
                current->score = do_score(current);
                retry = 0;
            }
            //runs and continues to run long as player continues selecting Hit
            else if(strcmp(option, "Hit") == 0 || strcmp(option, "hit") == 0 || strcmp(option, "HIT") == 0 || strcmp(option, "H") == 0 || strcmp(option, "h") == 0)
            {
                printf("Your card:\n");
                while(retry == 1)
                {
                    char ans[4];
                    int card_pos[2];
                    do
                    {
                        //assigns player a new card
                        random_card(card_pos);
                        current->cards[current->card_count] = cards[card_pos[0]][card_pos[1]];
                        current->suit[current->card_count] = card_pos[0];
                    } 
                    while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
                    int current_card = cards[card_pos[0]][card_pos[1]];
                    print_card(card_pos, current_card);
                    //removes selected card from deck
                    cards[card_pos[0]][card_pos[1]] = 0;
                    printf("\n");
                    current->card_count++;
                    //shows current hand
                    printf("Your current hand:\n");
                    see_hand(current);
                    //calculates new score for the current player
                    current->score = do_score(current);
                    //if player's score exceeds 21, player will bust and the loop will break
                    if(current->score > 21)
                    {
                        printf("%s Busted\n", current->name);
                        retry = 0;
                        sleep(2);
                        break;
                    }
                    //asks player if they would like another card
                    printf("Would you like another card? ");
                    scanf("%s", ans);
                    //if player wants to hit again, loop will continue
                    if(strcmp(ans, "Hit") != 0 && strcmp(ans, "hit") != 0 && strcmp(ans, "HIT") != 0 && strcmp(ans, "Yes") != 0 && strcmp(ans, "yes") != 0 && strcmp(ans, "YES") != 0 && strcmp(ans, "Y") != 0 && strcmp(ans, "y") != 0 && strcmp(ans, "H") != 0 && strcmp(ans, "h") != 0)
                    {
                        retry = 0;
                    }
                }
            }
            else
            {
                //if player inputs anything besides hit, double, or stand, error code will run
                printf("\nError reading input. Please try again.\n\n");
                sleep(1);
            }
        }
        printf("\n");
        //moves forward and reverses linked list
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
    //returns to the start of the list
    p = start_of_list(p, player_count);
    prev = NULL;
    next = NULL;
    current = p;
    for (int i = 0; i < player_count; i++)
    {
        printf("%s's cards:\n", current->name);
        //shows players hands
        see_hand(current);
        //moves forward and reverses linked list
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
    //returns back to the start of the list
    p = start_of_list(p, player_count);
    //shows dealers hand
    printf("Dealers's cards:\n");
    for(int j = 0; j < dealer->card_count; j++)
    {
        int current_card = dealer->cards[j];
        int card_pos[1] = {dealer->suit[j]};
        //prints dealers current hand
        print_card(card_pos, current_card);
        if(j < dealer->card_count - 1)
        {   
            printf(", ");
        }
    }
    //calculates dealer's score
    dealer->score = do_score(dealer);
    //if the dealer has less than 17 points, the dealer must hit
    while(dealer->score < 17)
    {
        printf(", ");
        int card_pos[2];
        do
        {
            //assigns dealer a new card
            random_card(card_pos);
            dealer->cards[dealer->card_count] = cards[card_pos[0]][card_pos[1]];
            dealer->suit[dealer->card_count] = card_pos[0];
        } 
        while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
        int current_card = cards[card_pos[0]][card_pos[1]];
        print_card(card_pos, current_card);
        //removes card from the deck
        cards[card_pos[0]][card_pos[1]] = 0;
        dealer->card_count++;
        //calculates dealer's score
        dealer->score = do_score(dealer);
    }
    printf("\n\n");
    prev = NULL;
    next = NULL;
    current = p;
    sleep(3);
    //runs if dealer exceeds a score of 21
    if(dealer->score > 21)
    {
        printf("Dealer Busted!\n");
    }
    for(int i = 0; i < player_count; i ++)
    {
        printf("********************\n");
        //1.5x bet is recieved if player's first two cards were equal to 21
        if(current->score == 21 && current->card_count == 2)
        {
            printf("%s, You Won!\n", current->name);
            current->bank += (current->bet * 1.5);
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        //runs if dealer busts, but player didn't
        else if(dealer->score > 21 && current->score <= 21)
        {
            printf("%s, You Win!\n", current->name);
            current->bank += (current->bet * 2);
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        //runs if player busted
        else if(current->score > 21)
        {
            printf("%s, You busted. Better Luck Next Time.\n", current->name);
            current->bet = 0; 
            printf("Your Bank: $%i\n", current->bank);
        }
        //runs if player beat the dealer
        else if(dealer->score < current->score)
        {
            printf("%s, You Win!\n", current->name);
            current->bank += (current->bet * 2);
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        //runs if player and dealer tie
        else if(dealer->score == current->score)
        {
            printf("%s, You tied with the Dealer.\n", current->name);
            current->bank += current->bet;
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        //player loses in any other situation
        else
        {
            printf("%s, You Lost. Better Luck Next Time.\n", current->name);
            current->bet = 0; 
            printf("Your Bank: $%i\n", current->bank);
        }
    //opens player.txt for editing
    player_data = fopen("players.txt", "r+");
    if (player_data == NULL)
    {
        //handle file opening error
        printf("Error opening file.\n");
        return 1;
    }
   FILE *temp_file = fopen("temp_players.txt", "w");
    if (temp_file == NULL)
    {
        //handle file opening error
        printf("Error opening file(s).\n");
        return 1;
    }
    char buffer[25];
    int player_found = 0;
    while (fgets(buffer, sizeof(buffer), player_data) != NULL)
    {
        //runs if player's name is found in the file
        if (strstr(buffer, current->name) != NULL) {
            //write the updated data to the new file
            fprintf(temp_file, "%s\n%d\n", current->name, current->bank);
            fgets(buffer, sizeof(buffer), player_data);
            player_found = 1;
        } 
        else 
        {
            //copy unchanged data to the new file
            fputs(buffer, temp_file);
        }
    }
    //closes both files
    fclose(player_data);
    fclose(temp_file);
    //remove the old file and rename the new file
    remove("players.txt");
    rename("temp_players.txt", "players.txt");
    //if the player was not found in the original file, append to the new file
    if (!player_found) {
        temp_file = fopen("players.txt", "a");
        if (temp_file != NULL) {
            fprintf(temp_file, "%s\n%d\n", current->name, current->bank);
            fclose(temp_file);
        } 
        //runs if there is an error opening the file
        else {
            printf("Error opening file for append.\n");
        }
    }
    //moves forward and reverses linked list
    next = current->next;
    current->next = prev;
    prev = current;
    current = next;
    }
    p = prev;
    //returns to the start of the list
    p = start_of_list(p, player_count);
    //frees each struct for player and dealer
    free_players(p);
    free(dealer);
}
//assigns each players name and score
player *assign_player(int player_count, FILE *player_data)
{
    char name[25];
    char buffer[25];
    char pulled_bank[7];
    int keep_reading = 1;
    int counter = 0;
    int bank = 1000;
    //mallocs memory for struct all_players
    player *all_players = (player*)malloc(sizeof(player));
    player *next;
    //runs if error occurs
    if(all_players == NULL)
    {
        printf("Error generating players.\nSystem shutting down...\n");
        return all_players;
    }
    if (player_count > 1)
    {
        //calls assign_player to get next player
        next = assign_player(player_count - 1, player_data);
    }
    printf("Enter player %i name: ", player_count);
    scanf("%s", name);
    //searches player.txt to find if the player that was entered already exists
    do
    {
        fgets(buffer, 25, player_data);
        //runs if end of file is reached
        if(feof(player_data))
        {
            keep_reading = 0;
            fseek(player_data, 0L, SEEK_SET);
        }
        //runs if player is found within the file
        else if ((strstr(buffer, name)) != NULL)
        {
            //gets player's bank from file
            fgets(pulled_bank, 7, player_data);
            bank = atoi(pulled_bank);
            keep_reading = 0;
            //sets player's bank to 100 if they have less than $20
            if(bank < 20)
            {
                bank = 100;
            }
            fseek(player_data, 0L, SEEK_SET);
        }
    } while (keep_reading == 1);
    //stores entered player name into all_players->name
    for(int i = 0; i < strlen(name); i++)
    {
        all_players->name[i] = name[i];
    }
    //all_player->next = next if player_count > 1
    if (player_count > 1)
    {
            all_players->next = next;
    }
    //else node for next = NULL
    else {
        all_players->next = NULL;
    }
    //assigns player bank and score
    all_players->bank = bank;
    all_players->score = 0;
    return all_players;
}
//prints player names and scores
void print_scores(player *p, int player_count)
{
    //next stores the next player in the linked list
    player *next = p->next;
    //prints scores
    printf("%s: %i\n", p->name, p->score);
    printf("Current bank: $%i\n", p->bank);
    //if there is no next player, return
    if(p->next == NULL)
    {
        return;
    }
    //runs print_score for next player in linked list
    print_scores(next, player_count - 1);
    return;
}
//randomly selects a card
void random_card(int card_pos[2])
{
    srand(time(0));
    //upper bound of possbile range
    int upper[2] = {14, 4};
    //lower bound of possible range
    int lower[2] = {2, 1};
    //randmly selects card suit
    card_pos[0] = lower[1] + (rand() % (upper[1] - lower[1] + 1));
    //randomly selects card number
    card_pos[1] = lower[0] + (rand() % (upper[0] - lower[0] + 1));
}
//returns linked list back to the start of the list
player *start_of_list(player *p, int player_count)
{
    player *prev = NULL;
    player *next = NULL;
    player *current = p;
for (int i = 0; i < player_count; i++)
    {
        /* 
        next will store the next player in the linked list
        node pointing to the next player, current->next, will store prev
        prev will store the current player
        current will go to the next player in the linked list
        This will move forward in the list and reverse the list
        */
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}
void print_card(int card_pos[], int current_card)
{
    //will print the cards suit
    if(card_pos[0] == 1)
    {
        printf("♡ ");
    }
    else if(card_pos[0] == 2)
    {
        printf("♢ ");
    }
    else if(card_pos[0] == 3)
    {
        printf("♤ ");
    }
    else
    {
        printf("♧ ");
    }
    //Will print the card as an ace, king, queen, jack, or its value
    if(current_card == A)
    {
        printf("A");
    }
    else if(current_card == K)
    {
        printf("K");
    }
    else if(current_card == Q)
    {
        printf("Q");
    }
    else if(current_card == J)
    {
        printf("J");
    }
    else
    {
        printf("%i", current_card);
    }
}
//shows player's current hand
void see_hand(player *p)
{
        for(int i = 0; i < p->card_count; i++)
        {
            int current_card = p->cards[i];
            int card_pos[1] = {p->suit[i]};
            print_card(card_pos, current_card);
            if(i < p->card_count - 1)
            {
                printf(", ");
            }
        }
        printf("\n");
}
//calculates player's score
int do_score(player *p)
{
    int score = 0;
    int temp_score;
    int check = 0;
    for(int i = 0; i < p->card_count; i++)
    {
        //temporarily calculates ace cards as a value of 1
        if(p->cards[i] == A)
        {
            temp_score = 1;
            check++;
        }
        //face cards are valued at 10
        else if(p->cards[i] > 10)
        {
            temp_score = 10;
        }
        else
        {
            temp_score = p->cards[i];
        }
        //adds temp_score to score
        score += temp_score;
    }
    /*
    Ace can be 1 or 11 in blackjack, but no one's going to pick ace to be 11 if it means they bust
    If player has an ace and it being worth 11 doesn't cause the player to bust, ace will be 11
    Since Ace was already determined to be 1, 10 will be added if the player doesn't bust by adding 10
    */
    if(check > 0 && score + 10 <= 21)
    {
        score += 10;
    }
    return score;
    
}
//frees each struct for players
void free_players(player *p)
{
    player *ptr = p;
    //If ptr isn't NULL if statement will run
    if (ptr != NULL)
    {
        //next will store the next player in the linked list
        player *next = ptr->next;
        //runs free_player() to free the stored player
        free_players(next);
    }
    //frees struct for player
    free(ptr);
    return;
}