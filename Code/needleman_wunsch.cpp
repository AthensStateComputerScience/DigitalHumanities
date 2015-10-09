#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

int main()
{
    /*This is test for input strings
     *The strings will come into the
     *Needleman_Wunsch Algorithm as
     *referenced to a1 and b1
    */
    string a1 =   "GGATCGA";
    string b1 =   "GAATTCAGTTA";


//SHOULD BE ABLE TO COPY FROM HERE DOWN TO MOVE INTO OUR PROGRAM
    /*The alpha grid is used to check
     *for matches in the given string
    */
    const size_t alphabets = 26;
    int alpha[alphabets][alphabets];

    string AlignmentA, AlignmentB;

    int gap_penalty = 0;            //THIS IS THE ALLOCATED SCORE FOR THE GAP PENALTY

    for (size_t i = 0; i < alphabets; i++)
    {
        for (size_t j = 0; j < alphabets; j++)
        {
            if (i == j)
            {
                alpha[i][j] = 1;    //THIS IS THE ALLOCATED SCORE FOR A MATCH
            }
            else
            {
                alpha[i][j] = 0;   //THIS IS THE ALLOCATED SCORE FOR A MISMATCH
            }
        }
    }

    size_t n = a1.size();
    size_t m = b1.size();

    vector<vector<int> > A(n + 1, vector<int>(m + 1));

    for (size_t i = 0; i <= m; ++i)
    {
        A[0][i] = 0;       //gap_Pen * i;
    }
    for (size_t j = 0; j <= n; ++j)
    {
        A[j][0] = 0;       //gap_Pen * j;
    }

    for (size_t i = 1; i <= n; ++i) //n
    {
        for (size_t j = 1; j <= m; ++j)  //m
        {
            char x_i = a1[i-1];  // Assigns x_i and y_j a char from the string
            char y_j = b1[j-1];

            int Match = A[i-1][j-1] + alpha[x_i -'A'][y_j - 'A'];
            int Delete= A[i-1][j] + gap_penalty;
            int Insert= A[i][j-1] + gap_penalty;

            if (Match >= Delete && Match >= Insert)
            {
                A[i][j] = Match;
            }
            else if (Delete >= Match && Delete >= Insert)
            {
                A[i][j] = Delete;
            }
            else
            {
                A[i][j] =  Insert;
            }
        }
    }
    AlignmentA = "";
    AlignmentB = "";
    size_t j = m;
    size_t i = n;

    for (; i >= 1 && j >= 1; --i)
    {
        char x_i = a1[i-1];
        char y_j = b1[j-1];
        if (A[i][j] == A[i-1][j-1] + alpha[x_i - 'A'][y_j - 'A'])
        {
            AlignmentA = x_i + AlignmentA;
            AlignmentB = y_j + AlignmentB;
            --j;
        }
        else if (A[i][j] == A[i-1][j] + gap_penalty)
        {
            AlignmentA = x_i + AlignmentA;
            AlignmentB = '-' + AlignmentB;
        }
        else
        {
            AlignmentA = '-' + AlignmentA;
            AlignmentB = y_j + AlignmentB;
            --j;
        }
    }

    while (i >= 1 && j < 1)
    {
        AlignmentA = a1[i-1] + AlignmentA;
        AlignmentB = '-' + AlignmentB;
        --i;
    }
    while (j >= 1 && i < 1)
    {
        AlignmentA = '-' + AlignmentA;
        AlignmentB = b1[j-1] + AlignmentB;
        --j;
    }
    int score = A[n][m];
    //return score;    //This should ultimately return score back to the call in our program

//FROM HERE DOWN CAN BE DELETED AND NOT PLACE INTO OUR PROJECT
    cout << "a: " << a1 << endl;
    cout << "b: " << b1 << endl;
    cout << "Needleman-Wunsch Score: " << score << endl;
    return 0;
}






