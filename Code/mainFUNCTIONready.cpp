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
#include <iomanip>
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


// FUNCTION PROTOTYPES --------------------------------------------------------------------------
string getInputDir();
void openInput(const string&);
void levenshteins(vector<string>&);
size_t levenshteinDistance(const string&, const string&);
void jaroWinkler(vector<string>&);
double jaroWinklerDistance(const string&, const string&);
void huntMcIlroy(vector<string>&);
double huntMcIlroyDistance(const string&, const string&);
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
	cout << "The amount of time used for this method is " << computedTime << " seconds" << endl << endl;
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
			cout << "file " << i + 1 << " -> file " << j + 1 << ": " << result << endl;
			output << result << ", ";
		}
	}

	//compute the time used and show it
	double computedTime = time.elapsed();
	cout << "The amount of time used for this method is " << computedTime << " seconds" << endl << endl;
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
	cout << "The amount of time used for this method is " << computedTime << " seconds" << endl << endl;
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
	cout << "The amount of time used for this method is " << computedTime << " seconds" << endl << endl;
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
double needlemanWunschDistance(const string&, const string&){
	// fill in
	return 0.0;
}


// huntMcIlroyDistance() - determines Hunt-McIlroy distance between 2 strings
double huntMcIlroyDistance(const string&, const string&){
	// fill in
	return 0.0;
}


// jaroWinklerDistance() - determines Jaro-Winkler distance between 2 strings
double jaroWinklerDistance(const string &s1, const string &s2){
	const int m(s1.size());
	const int n(s2.size());

	// if either string is blank, return 0.0
	if (min(m, n) == 0) return 0.0;

	// allowable is the maximum number of characters to check for a macth...
	// it's a range around the position of current char in the other string ...
	int allowable = int(floor(max(m, n)/2)) - 1;
	// matching is the number of matching characters among the strings
	int matching = 0;
	// half_t is 1/2 the number of transpositions (matching but different sequence order)
	int half_t = 0;



	/* min and max in algorithm, floor in cmath */
	// for each character in s1, match against each character of s2 within allowable distance floor(max(m,n)/2)-1



	return 101.01;
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





