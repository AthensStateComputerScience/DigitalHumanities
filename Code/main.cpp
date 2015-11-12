// Digital Humanities - Senior Project
// Program compares texts with various string editing metrics algorithms
// Authors: 
// --Kevin T. Woods
// --Greg Dawkins
// --Marlene Williams
// CS 452 - Dr. Adam Lewis (prof)


#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cctype>
#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <chrono>
using namespace std;

// GLOBALS --------------------------------------------------------------------------------------
ifstream input;
ofstream output;

//timer class to record computational time of the algorithms
class Timer
{
public:
	Timer() : beg_(clock_::now()) {}
	void reset() { beg_ = clock_::now(); }
	double elapsed() const {
		return std::chrono::duration_cast<second_>
			(clock_::now() - beg_).count();
	}

private:
	typedef std::chrono::high_resolution_clock clock_;
	typedef std::chrono::duration<double, std::ratio<1> > second_;
	std::chrono::time_point<clock_> beg_;
};

// to_upper forces use std::toupper rather than string::toupper
struct to_upper{
	int operator() (int ch){
		return std::toupper(ch);
	}
};

//This struct represents the coordinate values of the locations of k-matches
struct kCandidate {
	int coord1;
	int coord2;
};


// FUNCTION PROTOTYPES --------------------------------------------------------------------------
string getInputDir();
void openInput(const string&);
void levenshteins(vector<string>&);
size_t levenshteinDistance(const string&, const string&);
void jaroWinkler(vector<string>&);
const double jaroWinklerDistance(string, string);
const double jaroDistance(const int&, const int&, const int&, const int&);
void setJaroValues(const string&, const string&, const int&, int&, int&, const int&, const int&);
const int jaroCommonPrefix(const string&, const string&, const int&, const int&);
const double jaroPFX(const int&, const double&, const double&);
void huntMcIlroy(vector<string>&);
double huntMcIlroyDistance(const string&, const string&);
int huntMcIlroyAlg(kCandidate*, int, int, int);
void binSearch(kCandidate*, int, int, int);
void needlemanWunsch(vector<string>&);
double needlemanWunschDistance(const string&, const string&);
void getFilesInDirectory(vector<string>&, const string&);
void processFiles(vector<string>&, vector<string>&);
void compare(vector<string>&);
void setupOutput(vector<string>&);

// MAIN -----------------------------------------------------------------------------------------
int main()
{
	// declare variables
	vector<string> fileList, comparisonStrings;
	string inputDir;

	// get input directory from user
	inputDir = getInputDir();

	// get file list from input directory
	getFilesInDirectory(fileList, inputDir);

	// process the files, strip blank spaces, build strings vector
	processFiles(fileList, comparisonStrings);

	// open output file, place column headers
	setupOutput(fileList);

	// compare and get results
	compare(comparisonStrings);

	return 0;
}


// FUNCTION DEFINITIONS -------------------------------------------------------------------------
// General functions ------------------------------------------------

// openInput() - opens input file
void openInput(const string &fileName){	
	// open input file, exit on failure
	input.open(fileName.c_str());
	if (!input.is_open()){
		cerr << "ERROR: Failed to open input file: " << fileName;
		exit(EXIT_FAILURE);
	}
}


// setupOutput() - opens the output file and puts dynamic header in place
void setupOutput(vector<string>& fileList){
	//open the output file
	output.open("Result.csv");
	//print column headers for csv file
	output << "Method, ";
	int i = 0;
	for (std::vector<string>::iterator it = fileList.begin(); it != fileList.end(); ++it, ++i){
		int j = i + 1;
		for (std::vector<string>::iterator jt = it + 1; jt != fileList.end(); ++jt, ++j){
			output << i + 1 << " -> " << j + 1 << ", ";
		}
	}
	output << "Time" << endl;
}


// getInputDir() - returns path to input files
string getInputDir(){
	string dir = "";
	char reply;
	// loop to get dir name until user agrees on a valid dir
	while (true){
		cout << endl << "Enter the input file directory: " << endl;
		cin >> dir;
		struct stat statusInfo;
		if (stat(dir.c_str(), &statusInfo) != 0){ 
			// check if the path exists on the system
			cout << "ERROR: " << dir << " >> no such file or directory" << endl;
			cout << "Please try again" << endl;
		}
		else if (statusInfo.st_mode & S_IFDIR){ 
			// check if acutal dir name provided
			// display provided file dir path
			cout << endl << "Input directory: " << dir << endl;

			// check with user if dir is OK
			cout << "Is this correct? <Y/N>: ";
			cin >> reply;
			switch (reply){
			case 'Y':
			case 'y':
				break;
			default:
				// ignore and clear the cin buffer 
				cin.ignore(INT_MAX, '\n');
				cin.clear();
				// loop back to get new dir name
				continue;
			}
			return dir;
		}
		else{ // path to file name (not a dir) was given
			cout << "ERROR: " << dir << " >> is a file, not a directory" << endl;
			cout << "Please try again" << endl;
		}
	}
}

/* 
 *
 * the following file system directory listing function retrieved from
 * http://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c
 * credit goes to Andreas Bonini and lahjaton_j in their response and its edit
 * (slightly modified, works on windows ONLY)
 *
 */

// getFilesInDirectory() - returns list of input files to process (except the ones that begin with a dot)
void getFilesInDirectory(vector<string> &fileList, const string &directory){
	HANDLE dir;
	WIN32_FIND_DATA file_data;

	if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
		return; // No files found

	// display files to be processed and build the vector
	cout << endl << "Files to be processed: " << endl;
	do {
		const string file_name = file_data.cFileName;
		const string full_file_name = directory + "\\" + file_name;
		const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		if (file_name[0] == '.')
			continue;
		if (is_directory)
			continue;
		fileList.push_back(full_file_name);
		// display file name in cmd 
		cout << full_file_name << endl;
	} while (FindNextFile(dir, &file_data));
	cout << endl;
	FindClose(dir);
}


// processFiles() - handles the text extraction of provided files
void processFiles(vector<string>& files, vector<string>& texts){
	// open the files one by one and store their text in a string
	for (auto file : files){
		openInput(file);

		// store whole file in string
		string filetext = "";
		string line = "";
		while (getline(input, line)){
			// remove any whitespaces
			line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
			// append to previous lines
			filetext = filetext + line;
		}

		cout << filetext << endl << endl;

		// push it into the texts vector
		texts.push_back(filetext);

		// close file and reset stream
		input.close();
		input.clear();
	}
}


// void levenshteins() - loops through all files' strings and implementing levenshteinsDistance
void levenshteins(vector<string>& strings){
	//create an instance of the timer class
	Timer time;
	
	// loop through the strings comparing
	int i = 0;
	cout << "Levenshteins's Distance:" << endl;
	output << "Levenshtein's Distance, ";
	for (std::vector<string>::iterator it = strings.begin(); it != strings.end(); ++it, ++i){
		int j = i + 1;
		for (std::vector<string>::iterator jt = it + 1; jt != strings.end(); ++jt, ++j){
			size_t result = levenshteinDistance(strings[i], strings[j]);
			cout << "file " << i + 1 << " -> file " << j + 1 << ": " << result << endl;
			output << result << ", ";
		}
	}

	//compute the time used and show it
	double computedTime = time.elapsed();
	cout << "Time used for this method: " << computedTime << " seconds" << endl << endl;
	output << computedTime << endl;
}


// void jaroWinkler() - loops through all files' strings and implementing jaroWinklerDistance
void jaroWinkler(vector<string>& strings){
	//create an instance of the timer class
	Timer time;

	// loop through the strings comparing
	int i = 0;
	cout << "Jaro-Winkler Distance:" << endl;
	output << "Jaro-Winkler Distance, ";
	for (std::vector<string>::iterator it = strings.begin(); it != strings.end(); ++it, ++i){
		int j = i + 1;
		for (std::vector<string>::iterator jt = it + 1; jt != strings.end(); ++jt, ++j){
			double result = jaroWinklerDistance(strings[i], strings[j]);
			//double result = jaroWinklerDistance("DWAYNE", "DUANE");
			cout << "file " << i + 1 << " -> file " << j + 1 << ": " << result << endl;
			output << result << ", ";
		}
	}

	//compute the time used and show it
	double computedTime = time.elapsed();
	cout << "Time used for this method: " << computedTime << " seconds" << endl << endl;
	output << computedTime << endl;
}


// void huntMcIlroy() - loops through all files' strings and implementing huntMcIlroyDistance
void huntMcIlroy(vector<string>& strings){
	//create an instance of the timer class
	Timer time;

	// loop through the strings comparing
	int i = 0;
	cout << "Hunt-McIlroy Distance:" << endl;
	output << "Hunt-McIlroy Distance, ";
	for (std::vector<string>::iterator it = strings.begin(); it != strings.end(); ++it, ++i){
		int j = i + 1;
		for (std::vector<string>::iterator jt = it + 1; jt != strings.end(); ++jt, ++j){
			double result = huntMcIlroyDistance(strings[i], strings[j]);
			cout << "file " << i + 1 << " -> file " << j + 1 << ": " << result << endl;
			output << result << ", ";
		}
	}

	//compute the time used and show it
	double computedTime = time.elapsed();
	cout << "Time used for this method: " << computedTime << " seconds" << endl << endl;
	output << computedTime << endl;
}


// void needlemanWunsch() - loops through all files' strings and implementing needlemanWunschDistance
void needlemanWunsch(vector<string>& strings){
	//create an instance of the timer class
	Timer time;

	// loop through the strings comparing
	int i = 0;
	cout << "Needleman-Wunsch Distance:" << endl;
	output << "Needleman-Wunsch Distance, ";
	for (std::vector<string>::iterator it = strings.begin(); it != strings.end(); ++it, ++i){
		int j = i + 1;
		for (std::vector<string>::iterator jt = it + 1; jt != strings.end(); ++jt, ++j){
			double result = needlemanWunschDistance(strings[i], strings[j]);
			cout << "file " << i + 1 << " -> file " << j + 1 << ": " << result << endl;
			output << result << ", ";
		}
	}

	//compute the time used and show it
	double computedTime = time.elapsed();
	cout << "Time used for this method: " << computedTime << " seconds" << endl << endl;
	output << computedTime << endl;
}


// compare() - provides the comparison through specified algorithm(s)
void compare(vector<string>& strings){

	// compare texts with each implemented algorithm
	levenshteins(strings);
	jaroWinkler(strings);
	huntMcIlroy(strings);
	needlemanWunsch(strings);

	output.close();
}

// STRING EDITING METRICS FUNCTIONS ---------------------------------


// needlemanWunschDistance() - determines Needleman-Wunsch distance between 2 strings
double needlemanWunschDistance(const string&a1, const string&b1){
	const size_t alphabets = 130;
	int alpha[alphabets][alphabets];

	string AlignmentA, AlignmentB;

	int gap_penalty = -2;            //THIS IS THE ALLOCATED SCORE FOR THE GAP PENALTY

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
				alpha[i][j] = -2;   //THIS IS THE ALLOCATED SCORE FOR A MISMATCH
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
			char x_i = a1[i - 1];  // Assigns x_i and y_j a char from the string
			char y_j = b1[j - 1];
			int x_i_a = (int)x_i;
			int y_j_b = (int)y_j;

			int Match = A[i - 1][j - 1] + alpha[x_i_a][y_j_b];
			int Delete = A[i - 1][j] + gap_penalty;
			int Insert = A[i][j - 1] + gap_penalty;

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
				A[i][j] = Insert;
			}
		}
	}
	AlignmentA = "";
	AlignmentB = "";
	size_t j = m;
	size_t i = n;

	for (; i >= 1 && j >= 1; --i)
	{
		char x_i = a1[i - 1];
		char y_j = b1[j - 1];
		int x_i_a = (int)x_i;
		int y_j_b = (int)y_j;
		if (A[i][j] == A[i - 1][j - 1] + alpha[x_i_a][y_j_b])
		{
			AlignmentA = x_i + AlignmentA;
			AlignmentB = y_j + AlignmentB;
			--j;
		}
		else if (A[i][j] == A[i - 1][j] + gap_penalty)
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
		AlignmentA = a1[i - 1] + AlignmentA;
		AlignmentB = '-' + AlignmentB;
		--i;
	}
	while (j >= 1 && i < 1)
	{
		AlignmentA = '-' + AlignmentA;
		AlignmentB = b1[j - 1] + AlignmentB;
		--j;
	}
	double score = A[n][m];
	return score;
}


//Hunt-McIlroy Algorithm
//H-A utilizes the longest common subsquence to create a new file; 
//however, we are going to use the length of the longest to return the value of the length.

// huntMcIlroyDistance() - determines Hunt-McIlroy distance between 2 strings
double huntMcIlroyDistance(const string& s1, const string& s2){
	//get size of files. For our purposes, each file has one line.
	const int m(s1.size());
	const int n(s2.size());

	//variables for iteration and storage
	int i, j, maxRow = 0;
	int kValue = 0;

	//find matching subsequences
	for (i = 0; i < m; i++) {
		for (j = 0; j < n; j++){
			while (s1[i] == s2[j]) {
				i++;
				j++;
				if (i > m || j > n)
					break;
				maxRow++;
			}

			if (kValue < maxRow){
				kValue = maxRow;
			}
			maxRow = 0;
		}
	}
	return (double)kValue;
}


// jaroWinklerDistance() - determines Jaro-Winkler distance between 2 strings
const double jaroWinklerDistance(string str1, string str2){
	const int m(str1.size());
	const int n(str2.size());
	// if either string is blank, return 0.0 immediately
	if (min(m, n) == 0) return 0.0;

	// change pass by value copies of strings to all uppercase with std::transform algorithm
	transform(str1.begin(), str1.end(), str1.begin(), to_upper());
	transform(str2.begin(), str2.end(), str2.begin(), to_upper());

	// set pre-defined constants for the algorithm
	const double p = 0.1;			// constant scaling factor p
	const double boost = 0.7;		// boost threshold, compared against calculated jaro score

	// initialize variables that should change
	int half_t = 0;					// number of calculated transpositions, adjusted in setJaroValues()
	int matches = 0;				// number of calculated matches, adjusted in setJaroValues()

	// calculate other values used in the final algorithm
	const int matchRange = int(floor(max(m, n) / 2)) - 1;
	setJaroValues(str1, str2, matchRange, matches, half_t, m, n);
	const double jaro = jaroDistance(matches, half_t, m, n);

	// return the calculated jaro-winkler distance
	if (jaro < boost)
		return jaro;
	const int l = jaroCommonPrefix(str1, str2, m, n);
	return jaro + jaroPFX(l, p, jaro);
}


// setJaroValues() - sets the number of matches and transpositions used in jaroDistance()
void setJaroValues( const string& s1, const string& s2, const int& range, 
					int& numMatches, int& halfTrans, const int& m, const int& n){
	int i = 0, j = 0, transpositions = 0, start = 0;
	int lo, hi;

	// setup match flag vectors
	vector<int> v1, v2;
	v1.resize(m, 0);
	v2.resize(n, 0);


	// set number of matches within range, setting flags
	for (i = 0; i < m; ++i){
		lo = ((i - range) < 1) ? 0 : i - range; 
		hi = ((i + range) >= n) ? n : i + range;
		for (j = lo; j < hi; j++){
			if (v2[j] != 1 && s1[i] == s2[j]){
				numMatches++;
				v1[i] = 1;
				v2[j] = 1;
				break; // only do this for the first matching character
			}
		}
	}

	// determine the number of transpositions
	i = j = start = 0;
	for (i = 0; i < m; i++){
		hi = ((i + range) >= n) ? n : i + 1 + range;
		if (v1[i] == 1){
			for (j = start; j < hi; j++){
				if (v2[j] == 1){
					start = j + 1;
					break;
				}
			}
			if (s1[i] != s2[j]) transpositions++;
		}
	}
	halfTrans = transpositions/2;
}


// jaroDistance() - returns the calculated jaro score
const double jaroDistance(const int& _m, const int& half_t, const int& m, const int& n){
	return (_m == 0) ? 0.0 : (1/3.0) *(_m / double(m) + _m / double(n) + (_m - half_t) / double(_m));
}


// jaroCommonPrefix() - determines the common prefix weight used in Jaro-Winkler score
const int jaroCommonPrefix(const string& s1, const string& s2, const int& m, const int& n){
	int l = 0;
	int prefixMax = min(min(m, n), 4);
	for (int icp = 0; icp < prefixMax; ++icp){
		if (s1[icp] == s2[icp])
			l++;
		else
			break;
	}

	return l;
}


// jaroPFX() - from the jaro-winkler algorithm, returns the value (lp(1-jaroDistance))
const double jaroPFX(const int& l, const double& p, const double& dj){
	return l*p*(1 - dj);
}

/*
 * 
 * the following implementation of Levenshtein's distance algorithm 
 * was provided by Greg Dawkins and his previous class' group with Ms. Mayfield
 *
 */

// levenshteinDistance() - determines Levenshtein's distance metric between 2 strings
size_t levenshteinDistance(const string &s1, const string &s2){
	const size_t m(s1.size());
	const size_t n(s2.size());

	if (m == 0) return n;
	if (n == 0) return m;

	size_t *costs = new size_t[n + 1];

	for (size_t k = 0; k <= n; k++) costs[k] = k;

	size_t i = 0;
	for (string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i)
	{
		costs[0] = i + 1;
		size_t corner = i;

		size_t j = 0;
		for (string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j)
		{
			size_t upper = costs[j + 1];
			if (*it1 == *it2)
			{
				costs[j + 1] = corner;
			}
			else
			{
				size_t t(upper<corner ? upper : corner);
				costs[j + 1] = (costs[j]<t ? costs[j] : t) + 1;
			}

			corner = upper;
		}
	}

	size_t result = costs[n];
	delete[] costs;

	return result;
}





