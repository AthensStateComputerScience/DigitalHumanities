#include <string>
#include <iostream>
#include <fstream>
#include <cctype>

using namespace std;

ifstream inputFile;


size_t uiLevenshteinDistance(const std::string &s1, const std::string &s2)
{
  const size_t m(s1.size());
  const size_t n(s2.size());

  if( m==0 ) return n;
  if( n==0 ) return m;

  size_t *costs = new size_t[n + 1];

  for( size_t k=0; k<=n; k++ ) costs[k] = k;

  size_t i = 0;
  for ( std::string::const_iterator it1 = s1.begin(); it1 != s1.end(); ++it1, ++i )
  {
    costs[0] = i+1;
    size_t corner = i;

    size_t j = 0;
    for ( std::string::const_iterator it2 = s2.begin(); it2 != s2.end(); ++it2, ++j )
    {
      size_t upper = costs[j+1];
      if( *it1 == *it2 )
      {
		  costs[j+1] = corner;
	  }
      else
	  {
		size_t t(upper<corner?upper:corner);
        costs[j+1] = (costs[j]<t?costs[j]:t)+1;
	  }

      corner = upper;
    }
  }

  size_t result = costs[n];
  delete [] costs;

  return result;
}

void openInputFile1()
    {
        inputFile.open ("Poem3.txt");
        if(! inputFile)
        {
            cout<<"Error Opening Input File #1"<<endl;
        }
    }

 void openInputFile2()
    {
        inputFile.open ("Poem4.txt");
        if(! inputFile)
        {
            cout<<"Error Opening Input Poem #2"<<endl;
        }
    }

 void openInputFile3()
    {
        inputFile.open ("Poem5.txt");
        if(! inputFile)
        {
            cout<<"Error Opening Input Poem #2"<<endl;
        }
    }

int main()
{
    openInputFile1();

    string s1,s2,s3;
    string poem1,poem2,poem3;


    while (inputFile)           //Read Poem 1
    {
        getline(inputFile,s1);
        poem1 = poem1 + s1;
    }
    cout<<poem1<<endl<<endl;
    inputFile.close();

    openInputFile2();           //Read Poem 2
    while (inputFile)
    {
        getline(inputFile,s2);
        poem2 = poem2 + s2;
    }
    cout<<poem2<<endl<<endl;
    inputFile.close();

    openInputFile3();           //Read Poem 3
    while (inputFile)
    {
        getline(inputFile,s3);
        poem3 = poem3 + s3;
    }
    cout<<poem3<<endl<<endl;
    inputFile.close();

	cout<< "The Distance between Poem 1 and Poem 2 is " << uiLevenshteinDistance(poem1,poem2) <<endl;
	cout<< "The Distance between Poem 1 and Poem 3 is " << uiLevenshteinDistance(poem1,poem3) <<endl;
	cout<< "The Distance between Poem 2 and Poem 3 is " << uiLevenshteinDistance(poem2,poem3) <<endl;

        return 0;
}
