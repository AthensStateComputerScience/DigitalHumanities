// Digital Humanities - Senior Project
// Program compares texts with various string editing metrics algorithms
// Authors: 
// --Kevin T. Woods
// --Greg Dawkins
// --Marlene Williams
// CS 452 - Dr. Adam Lewis (prof)


#include <string>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <Windows.h>
#include <chrono>
using namespace std;

// GLOBALS --------------------------------------------------------------------------------------
ifstream input;

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
size_t levenshteinDistance(const string&, const string&);
void getFilesInDirectory(vector<string>&, const string&);
void processFiles(vector<string>&, vector<string>&);
void compare(vector<string>&);

// MAIN -----------------------------------------------------------------------------------------
int main()
{
	vector<string> fileList, comparisonStrings;
	string inputDir;

	//create the timer, compute used time after each call then reset the timer.
	Timer time;

	inputDir = getInputDir();
	getFilesInDirectory(fileList, inputDir);
	processFiles(fileList, comparisonStrings);
	compare(comparisonStrings);

	//get the computed time
	double computedTime = time.elapsed();
	//output the computed time
	cout << "The amount of time used for this method is " << computedTime << endl;
	//reset the timer for the next algorithm
	time.reset();

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


// compare() - provides the comparison through specified algorithm(s)
void compare(vector<string>& strings){
	// compare textx with each implemented algorithm
	cout << "Levenshtens's Distance:" << endl;
	int i = 0;
	for (std::vector<string>::iterator it = strings.begin(); it != strings.end(); ++it, ++i){
		int j = i + 1;
		for (std::vector<string>::iterator jt = it + 1; jt != strings.end(); ++jt, ++j){
			cout << "file " << i+1 << " -> file " << j+1 << ": " 
				<< levenshteinDistance(strings[i], strings[j]) << endl;

			// non-levenshteins algorithm a 
			// non-levenshteins algorithm b
			// non-levenshteins algorithm c
			// work on formatting and consider .csv table output
		}
	}
}

// STRING EDITING METRICS FUNCTIONS ---------------------------------

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



