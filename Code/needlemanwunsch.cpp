#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

const size_t alphabets = 26;   //used to create alpha grid
int needleman_wunsch(const string &a, const string &b, int gap_Pen, int alpha[alphabets][alphabets], string &AlignmentA, string &AlignmentB);
int max(int a, int b, int c);

////******************************************************************************************************
////*************************NEEDS TO GO IN THE MAIN PROGRAM**********************************************
////******************************************************************************************************
int main()
{
    // THIS IS WHERE WE WILL NEED TO SET THE INPUT EQUAL TO THE INPUT FROM POEMS**************************
    // The strings below are used for testing
            /*  Setting the Match, Mismatch, Gap Penalty changes the Score
             *  These can be changed on lines 36,44,48
             *  The Gap Penalty looks at the total gaps in the sequence
            */
    string a1 =   "GGATCGA";
    string b1 =   "GAATTCAGTTA";

    int alpha[alphabets][alphabets];
    string a2, b2;

    /* Creates a grid for alphabets in Rows and Columns
     * These are in a grid of i and j
     * If i and j are equal (ex. Row 4 and Column 4)
     * then alpha [4][4] = 2;
    */

    int gap_penalty = -2;            //THIS IS THE ALLOCATED SCORE FOR THE GAP PENALTY

    for (size_t i = 0; i < alphabets; i++)
    {
        for (size_t j = 0; j < alphabets; j++)
        {
            if (i == j)
            {
                alpha[i][j] = 2;    //THIS IS THE ALLOCATED SCORE FOR A MATCH
            }
            else
            {
                alpha[i][j] = -1;   //THIS IS THE ALLOCATED SCORE FOR A MISMATCH
            }
        }
    }

    int score = needleman_wunsch(a1, b1, gap_penalty, alpha, a2, b2);

    /* ONLY USED For Output to Console  */
    cout << "a: " << a1 << endl;
    cout << "b: " << b1 << endl;
    cout << "Needleman-Wunsch Score: " << score << endl;
    return 0;
}
////*******************************************************************************************************
////**************END MAIN PROGRAM*************************************************************************
////*******************************************************************************************************

int needleman_wunsch(const string &a, const string &b, int gap_Pen,int alpha[alphabets][alphabets], string &AlignmentA, string &AlignmentB)
{

    size_t n = a.size();    //Gets size of the 2 strings that is passed into Function
    size_t m = b.size();

    /* Creates a Vector the size of the strings for Comparing Cells */
    vector<vector<int> > A(n + 1, vector<int>(m + 1));

    ////*************CONSTRUCT AND ALIGN THE MATRIX / Compute the Matrix*****************************************

    //This sets up the 1st Row and Column of the Matrix based on the Gap Penalty
    for (size_t i = 0; i <= m; ++i)
    {
        A[0][i] = 0;       //gap_Pen * i;
    }
    for (size_t j = 0; j <= n; ++j)
    {
        A[j][0] = 0;       //gap_Pen * j;
    }

    //This fills the Matrix based on Comparing the string Text and adjoining
    //places (Up, Down, Diagonal) in the Matrix
    for (size_t i = 1; i <= n; ++i) //n
    {
        for (size_t j = 1; j <= m; ++j)  //m
        {
            char x_i = a[i-1];  // Assigns x_i and y_j a char from the string
            char y_j = b[j-1];

            //Calls the Max Function to return the highest score
            int Match = A[i-1][j-1] + alpha[x_i -'A'][y_j - 'A'];
            int Delete= A[i-1][j] + gap_Pen;
            int Insert= A[i][j-1] + gap_Pen;

            A[i][j] = max (Match, Delete, Insert);
        }
    }

    //*  PRINTS THE Above Matrix FOR TESTING
   /* for (int i = 0; i < n+1; i++)
        {
            for (int j = 0; j < m+1; j++)
            {
                cout <<  setw(5) << A[i][j];
            }
            cout<<endl<<endl;
        }
        cout << endl;
    */

////************************************Compute the Score*******************************************************
    AlignmentA = "";
    AlignmentB = "";
    size_t j = m;
    size_t i = n;

    for (; i >= 1 && j >= 1; --i)
    {
        char x_i = a[i-1];
        char y_j = b[j-1];
        if (A[i][j] == A[i-1][j-1] + alpha[x_i - 'A'][y_j - 'A'])
        {
            AlignmentA = x_i + AlignmentA;
            AlignmentB = y_j + AlignmentB;
            --j;
        }
        else if (A[i][j] == A[i-1][j] + gap_Pen)
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
        AlignmentA = a[i-1] + AlignmentA;
        AlignmentB = '-' + AlignmentB;
        --i;
    }
    while (j >= 1 && i < 1)
    {
        AlignmentA = '-' + AlignmentA;
        AlignmentB = b[j-1] + AlignmentB;
        --j;
    }
    return A[n][m];

}

int max(int a, int b, int c)
{
    if (a >= b && a >= c)
    {
        return a;
    }
    else if (b >= a && b >= c)
    {
        return b;
    }

    else
    {
        return c;
    }
}

