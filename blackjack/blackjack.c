#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define A 14
#define K 13
#define Q 12
#define J 11
#define ASK 0
#define DONT_ASK 1

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
    struct player *next;
}
player;

typedef struct saved_player
{
    char name[25];
    int bank;
}
saved_player;

player *assign_player(int player_count, FILE *player_data);
player *start_of_list(player *p, int player_count);

int do_score(player *p);

void print_scores(player *p, int player_count);
void free_players(player *p);
void see_hand(player *p, int ask);
void random_card(int card_pos[2]);
void print_card(int cards_pos[], int current_card);

int main(void)
{
    const int MIN_BET = 20;
    int cards[4][13] = {{2,3,4,5,6,7,8,9,10,J,Q,K,A}, 
                        {2,3,4,5,6,7,8,9,10,J,Q,K,A},
                        {2,3,4,5,6,7,8,9,10,J,Q,K,A},
                        {2,3,4,5,6,7,8,9,10,J,Q,K,A}};
    int player_count;
    player *dealer = malloc(sizeof(player));
    if(dealer == NULL)
    {
       printf("Error generating Dealer.\nSystem shutting down...\n");
        return 1;
    }
    printf("*******************************************\n");
    printf("*********  Welcome to Blackjack  **********\n");
    printf("*******************************************\n\n");
    printf("Enter amount of players: ");
    scanf("%i", &player_count);
    printf("\nKeep player names no larger than 25 characters\n");
    printf("Player names may also not include any spaces\n\n");
    //assign each players name and score into struct p
    FILE *player_data;
    player_data = fopen("players.txt", "r");
    if(player_data == NULL)
    {
        player_data = fopen("players.txt", "w");
        fclose(player_data);
        player_data = fopen("players.txt", "r");
        if(player_data == NULL)
        {
            printf("An Error Has Occured\nShutting System Down...\n");
            return 1;
        }
    }
    player *p = assign_player(player_count, player_data);
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
        int bank;
        do
        {
            bank = current->bank;
            printf("%s place your bet:\n", current->name);
            scanf("%i", &bet);
            bank -= bet;
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
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
    //return back to start of list p
    p = start_of_list(p, player_count);
    printf("\n");
    //assigns starting cards to players
    current = p;
    prev = NULL;
    next = NULL;
    for(int i = 0; i < player_count; i++)
    {
        printf("%s's cards:\n", current->name);
        for(int j = 0; j < 2; j++)
        {
            int card_pos[2];
            do
            {
                random_card(card_pos);
                current->cards[j] = cards[card_pos[0]][card_pos[1]];
                current->suit[j] = card_pos[0];
            } 
            //sometimes resulted in numbers outside of the bounds
            //Set bounds in while loop to always be within bounds
            while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
            int current_card = cards[card_pos[0]][card_pos[1]];
            print_card(card_pos, current_card);
            if(j == 0)
            {
                printf(", ");
            }
            cards[card_pos[0]][card_pos[1]] = 0;
        }
        current->card_count = 2;
        current->score = do_score(current);
        printf("\n");
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
            random_card(card_pos);
            dealer->cards[j] = cards[card_pos[0]][card_pos[1]];
            dealer->suit[j] = card_pos[0];
        } 
        while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
        int current_card = cards[card_pos[0]][card_pos[1]];
        //prints dealer's first card(face up)
        //second card is not shown(face down)
        if(j == 0)
        {
            print_card(card_pos, current_card);
            printf(", ");
        }
        else
        {
            printf("?\n");
        }
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
        while(retry == 1)
        {
            if(current->score == 21)
            {
                printf("%s, You've already won with a score of 21. You get 1.5x your bet\n", current->name);
                retry = 0;
                continue;
            }
            printf("%s these are your cards:\n", current->name);
            see_hand(current, DONT_ASK);
            printf("Please select one of the options below\n");
            printf("Hit, Pass");
            if(current->bank >= current->bet)
            {
                printf(", Double");
            }
            printf("\n");
            scanf("%s", option);
            //runs if player chooses to Pass
            if(strcmp(option, "Pass") == 0 || strcmp(option, "pass") == 0 || strcmp(option, "PASS") == 0)
            {
                retry = 0;
            }
            //runs if player chooses to Double Down
            else if(strcmp(option, "Double") == 0 || strcmp(option, "double") == 0 || strcmp(option, "DOUBLE") == 0)
            {
                if(current->bank < current->bet)
                {
                    printf("\nYou don't have enough in your bank to Double Down\nPlease try again\n\n");
                    sleep(2);
                    continue;
                }
                int bet = current->bet;
                int new_bet = bet * 2;
                current->bet = new_bet;
                printf("Your new bet is: %i\n", current->bet);
                printf("Your card:\n");
                int card_pos[2];
                do
                {
                    random_card(card_pos);
                    current->cards[2] = cards[card_pos[0]][card_pos[1]];
                    current->suit[2] = card_pos[0];
                } 
                while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
                current->card_count++;
                int current_card = cards[card_pos[0]][card_pos[1]];
                print_card(card_pos, current_card);
                cards[card_pos[0]][card_pos[1]] = 0;
                printf("\n");
                see_hand(current, ASK);
                current->score = do_score(current);
                retry = 0;
            }
            //runs and continues to run long as player continues selecting Hit
            else if(strcmp(option, "Hit") == 0 || strcmp(option, "hit") == 0 || strcmp(option, "HIT") == 0)
            {
                printf("Your card:\n");
                while(retry == 1)
                {
                    char ans[4];
                    int card_pos[2];
                    do
                    {
                        random_card(card_pos);
                        current->cards[current->card_count] = cards[card_pos[0]][card_pos[1]];
                        current->suit[current->card_count] = card_pos[0];
                    } 
                    while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
                    int current_card = cards[card_pos[0]][card_pos[1]];
                    print_card(card_pos, current_card);
                    cards[card_pos[0]][card_pos[1]] = 0;
                    printf("\n");
                    current->card_count++;
                    see_hand(current, ASK);
                    current->score = do_score(current);
                    if(current->score > 21)
                    {
                        printf("%s Busted\n", current->name);
                        retry = 0;
                        sleep(2);
                        break;
                    }
                    printf("Would you like another card? ");
                    scanf("%s", ans);
                    if(strcmp(ans, "Hit") != 0 && strcmp(ans, "hit") != 0 && strcmp(ans, "HIT") != 0 && strcmp(ans, "Yes") != 0 && strcmp(ans, "yes") != 0 && strcmp(ans, "YES") != 0 && strcmp(ans, "Y") != 0 && strcmp(ans, "y") != 0)
                    {
                        retry = 0;
                    }
                }
            }
            else
            {
                printf("\nError reading input. Please try again.\n\n");
                sleep(1);
            }
        }
        printf("\n");
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
    p = start_of_list(p, player_count);
    prev = NULL;
    next = NULL;
    current = p;
    for (int i = 0; i < player_count; i++)
    {
        printf("%s's cards:\n", current->name);
        see_hand(current, DONT_ASK);
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
    p = start_of_list(p, player_count);
    printf("Dealers's cards:\n");
    for(int j = 0; j < dealer->card_count; j++)
    {
        int current_card = dealer->cards[j];
        int card_pos[1] = {dealer->suit[j]};
        print_card(card_pos, current_card);
        if(j < dealer->card_count - 1)
        {   
            printf(", ");
        }
    }
    dealer->score = do_score(dealer);
    while(dealer->score < 17)
    {
        printf(", ");
        int card_pos[2];
        do
        {
            random_card(card_pos);
            dealer->cards[dealer->card_count] = cards[card_pos[0]][card_pos[1]];
            dealer->suit[dealer->card_count] = card_pos[0];
        } 
        while (cards[card_pos[0]][card_pos[1]] < 2 || cards[card_pos[0]][card_pos[1]] > 14);
        int current_card = cards[card_pos[0]][card_pos[1]];
        print_card(card_pos, current_card);
        cards[card_pos[0]][card_pos[1]] = 0;
        dealer->card_count++;
        dealer->score = do_score(dealer);
    }
    printf("\n\n");
    prev = NULL;
    next = NULL;
    current = p;
    saved_player *temp = malloc(sizeof(saved_player));
    sleep(3);
    if(dealer->score > 21)
    {
        printf("Dealer Busted!\n");
    }
    for(int i = 0; i < player_count; i ++)
    {
        printf("********************\n");
        if(current->score == 21 && current->card_count == 2)
        {
            printf("%s, You Won!\n", current->name);
            current->bank += (current->bet * 1.5);
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        else if(dealer->score > 21 && current->score <= 21)
        {
            printf("%s, You Win!\n", current->name);
            current->bank += (current->bet * 2);
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        else if(current->score > 21)
        {
            printf("%s, You busted. Better Luck Next Time.\n", current->name);
            current->bet = 0; 
            printf("Your Bank: $%i\n", current->bank);
        }
        else if(dealer->score < current->score)
        {
            printf("%s, You Win!\n", current->name);
            current->bank += (current->bet * 2);
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        else if(dealer->score == current->score)
        {
            printf("%s, You tied with the Dealer.\n", current->name);
            current->bank += current->bet;
            current->bet = 0;
            printf("Your Bank: $%i\n", current->bank);
        }
        else
        {
            printf("%s, You Lost. Better Luck Next Time.\n", current->name);
            current->bet = 0; 
            printf("Your Bank: $%i\n", current->bank);
        }
        player_data = fopen("players.txt", "a");
        fprintf(player_data, "%s\n%i\n", current->name, current->bank);
        fclose(player_data);
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    p = prev;
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
    player *all_players = malloc(sizeof(player));
    if(all_players == NULL)
    {
        printf("Error generating players.\nSystem shutting down...\n");
        return all_players;
    }
    if (player_count > 1)
    {
        player *next = assign_player(player_count - 1, player_data);
        printf("Enter player %i name: ", player_count);
        scanf("%s", name);
       do
        {
            fgets(buffer, 25, player_data);
            if(feof(player_data))
            {
                keep_reading = 0;
                fseek(player_data, 0L, SEEK_SET);
            }
            else if ((strstr(buffer, name)) != NULL)
            {
                fgets(pulled_bank, 7, player_data);
                bank = atoi(pulled_bank);
                keep_reading = 0;
                if(bank == 0)
                {
                    bank = 1000;
                }
                fseek(player_data, 0L, SEEK_SET);
            }
        } while (keep_reading == 1);
        for(int i = 0; i < strlen(name); i++)
        {
            all_players->name[i] = name[i];
        }
        all_players->next = next;
        all_players->bank = bank;
        all_players->score = 0;
    }
    else
    {
        printf("Enter player %i name: ", player_count);
        scanf("%s", name);
        do
        {
            fgets(buffer, 25, player_data);
            if(feof(player_data))
            {
                keep_reading = 0;
                fseek(player_data, 0L, SEEK_SET);
            }
            else if ((strstr(buffer, name)) != NULL)
            {
                fgets(pulled_bank, 7, player_data);
                bank = atoi(pulled_bank);
                keep_reading = 0;
                if(bank == 0)
                {
                    bank = 1000;
                }
                fseek(player_data, 0L, SEEK_SET);
            }
        } while (keep_reading == 1);
        
        for(int i = 0; i < strlen(name); i++)
        {
            all_players->name[i] = name[i];
        }
        all_players->next = NULL;
        all_players->bank = bank;
        all_players->score = 0;
    }
    return all_players;
}

//prints player names and scores
void print_scores(player *p, int player_count)
{
    player *next = p->next;
    printf("%s: %i\n", p->name, p->score);
    printf("Current bank: $%i\n", p->bank);
    if(p->next == NULL)
    {
        return;
    }
    print_scores(next, player_count - 1);
    return;
}

void random_card(int card_pos[2])
{
    srand(time(0));
    int upper[2] = {14, 4};
    int lower[2] = {2, 1};
    card_pos[0] = lower[1] + (rand() % (upper[1] - lower[1] + 1));
    card_pos[1] = lower[0] + (rand() % (upper[0] - lower[0] + 1));
}
player *start_of_list(player *p, int player_count)
{
    player *prev = NULL;
    player *next = NULL;
    player *current = p;
for (int i = 0; i < player_count; i++)
    {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

void print_card(int card_pos[], int current_card)
{
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
void see_hand(player *p, int ask)
{
    char answer[4];
    if(ask == ASK)
    {
        printf("Would you like to see your current hand? ");
        scanf("%s", answer);
    }
    if(strcmp(answer, "Yes") == 0 || strcmp(answer, "yes") == 0 || strcmp(answer, "YES") == 0 || strcmp(answer, "Y") == 0 || strcmp(answer, "y") == 0 || ask == DONT_ASK)
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
}
int do_score(player *p)
{
    int score = 0;
    int temp_score;
    int check = 0;
    for(int i = 0; i < p->card_count; i++)
    {
            if(p->cards[i] == A)
            {
                temp_score = 1;
                check++;
            }
            else if(p->cards[i] > 10)
            {
                temp_score = 10;
            }
            else
            {
                temp_score = p->cards[i];
            }
            score += temp_score;
    }
    /*
    Ace can be 1 or 11 in blackjack, but no one's going to pick ace to be 14 if it means they bust
    If player has an ace and it being worth 11 doesn't cause the player to bust, ace will be 11
    */
    if(check > 0 && score + 10 < 21)
    {
        score += 10;
    }
    return score;
    
}
//frees each struct for players
void free_players(player *p)
{
    player *ptr = p;
    if (ptr != NULL)
    {
        player *next = ptr->next;
        free_players(next);
    }
    free(ptr);
    return;
}