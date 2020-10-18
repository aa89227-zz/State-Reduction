#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <set>
#include <list>
#define MAX_COUNT 999
#define SEED 1001
#define TESTCASE 1001

#define SHOW_INPUT 0
#define SHOW_CMP 0
#define SHOW_ABLE 0
#define SHOW_REQ 0
#define SHOW_MERGE 0
#define SHOW_RESULT 1
#define MODE 0 //output 0, test 1
using namespace std;

using nextState = map< string, pair< string, string> >;

struct state
{
	string name; //state name
	nextState list; // input, (next-state, output)
	set< string > same;
};

int numInputs, numOutputs, numStates, numTerms;
string resetState;
map<string, state> stateMap;

// put state into stateMap
void inputState(string& line);

// find possible merge condition
void findPossible();

// merge 
void merge(vector< set<string> >& stateSet, map< string, state>::iterator pState, map<string, int> position);

// try to merge, return empty if fail to merge
void singleMerge(vector< set<string> >& stateSet, string l, string r, map<string,int>& position);

// cmp two result, return true if they are equal
bool isEqual(const vector< set<string> >& l, const vector< set<string> >& r);

// compare two state, return true if they are able to merge
bool cmpState(string l, string r);

// compare two binary code, return true if they are the same
bool cmpCode(string a, string b); 

// whole merge
void finishMap();

// output
void output();

// show result
void showResult();

// test
void test();

list< vector< set<string> > > listResult;

vector< map<string, state> > fullstateMap;

int minTerm;
int minTerm_pos;

int main() {
	string cmd, line;
	stringstream ss;
	string fileName;
	getline(cin, fileName);
	//ifstream inFile(argv[1], ios::in);
	ifstream inFile(fileName, ios::in);
	if ( !inFile ) {
		cout << "fail!" << endl;
		return 1;
	}
	while (getline(inFile, line) ) {
		if ( line.find('#') != string::npos ) continue; //µù¸Ñ

		ss.clear();
		ss.str(line);
		ss >> cmd;
		if ( cmd == ".i" ) ss >> numInputs;
		else if ( cmd == ".o" ) ss >> numOutputs;
		else if ( cmd == ".s" ) ss >> numStates;
		else if ( cmd == ".p" ) ss >> numTerms;
		else if ( cmd == ".r" ) ss >> resetState;
		else if ( cmd == ".e" ) break;
		else if ( line != "" ) inputState(line);

	}
	inFile.close();
	minTerm = numTerms;

#if SHOW_INPUT == 1
	cout << "-------------state map-------------";

	for ( map<string, state>::iterator it = stateMap.begin(); it != stateMap.end(); ++it ) {
		cout << endl << (*it).first << ": ";

		for ( nextState::iterator it2 = (*it).second.list.begin(); it2 != (*it).second.list.end(); ++it2 )
			cout << endl << (*it2).first << " " << (*it2).second.first << " " << (*it2).second.second;

		cout << endl;
	}
	cout << "---------------end-----------------" << endl;
#endif

	findPossible();
	vector<set<string>> nullVec;
	map<string, int> position;

#if SHOW_MERGE == 1
	cout << endl << "====================== start merge ======================" << endl;
#endif

	merge(nullVec, stateMap.begin(), position);

#if SHOW_MERGE == 1
	cout << endl << "====================== end merge ======================" << endl;
#endif 
	finishMap();

#if SHOW_RESULT == 1
	showResult();
#endif

#if MODE == 0
	output();
#else
	test();
#endif
}

void inputState(string& line )
{
	string input, output;
	string name, next;
	stringstream ss(line);
	ss >> input >> name >> next >> output;
	stateMap[name].name = name;
	stateMap[name].list[input] = pair<string, string>(next, output);
}

map< pair<string, string>, set<pair<string, string>> > root;// (a,b) -> (A,B), (C,D)
set<pair<string, string>> banedRoot;
void findPossible()
{
	for ( map<string, state>::iterator it1 = stateMap.begin(); it1 != stateMap.end(); ++it1 ) { //create relative graph
		for ( map<string, state>::iterator it2 = (++it1)--; it2 != stateMap.end(); ++it2 ) {
			if ( (*it1).second.same.find((*it2).first) != (*it1).second.same.end() ) continue;
#if SHOW_CMP == 1
			cout << "\n----------- start compare " << (*it1).first << " and " << (*it2).first << " ----------- " << endl;
#endif
			if ( cmpState((*it1).first, (*it2).first) ) {
				(*it1).second.same.insert((*it2).first);
				(*it2).second.same.insert((*it1).first);
			}
			else {
				banedRoot.insert(pair<string, string>((*it1).first, (*it2).first));
#if SHOW_CMP == 1 
				cout << "false" << endl;
#endif
			}
		}
	}
#if SHOW_ABLE == 1 
	cout << endl;
	cout << "-------------A can merge with {B}-------------" << endl;
	for ( map<string, state>::iterator it1 = stateMap.begin(); it1 != stateMap.end(); ++it1 ) {
		cout << (*it1).first << " :";
		for ( set<string>::iterator it2 = (*it1).second.same.begin(); it2 != (*it1).second.same.end(); ++it2 )
			cout << " " << *it2;
		cout << endl;
	}
	cout << "----------------------end-----------------------" << endl;
#endif
#if SHOW_REQ == 1
	cout << endl;
	cout << "-------------merger requirement-------------" << endl;
	for ( map< pair<string, string>, set<pair<string, string>> >::iterator it1 = root.begin(); it1 != root.end(); ++it1 ) {
		if ( banedRoot.find(pair<string, string>((*it1).first.first, (*it1).first.second)) != banedRoot.end() ) continue;
		cout << ">( " << (*it1).first.first << " , " << (*it1).first.second << " ) : ";
		for ( set<pair<string, string>>::iterator it2 = (*it1).second.begin(); it2 != (*it1).second.end(); ++it2 ) {
			cout << "( " << (*it2).first << " , " << (*it2).second << " ) ";
		}
		cout << endl;
	}
	cout << "----------------------end-----------------------" << endl;
#endif
	
}

void merge(vector<set<string>>& stateSet, map< string, state>::iterator pState, map<string, int> position)
{
	static int minResult = numStates;
	if ( pState == stateMap.end() ) { // finished

		if ( stateSet.size() > minResult ) return;
		if ( stateSet.size() < minResult ) listResult.clear();
		minResult = stateSet.size();

		for ( list<vector<set<string>>>::iterator it = listResult.begin(); it != listResult.end(); ++it )
			if ( isEqual(stateSet, *it) ) return;

		listResult.push_back(stateSet);
#if SHOW_MERGE == 1
		cout << "----------------- with " << minResult << " states -----------------" << endl;
		for ( vector<set<string>>::iterator it = stateSet.begin(); it != stateSet.end(); ++it ) {
			for ( set<string>::iterator it1 = (*it).begin(); it1 != (*it).end(); ++it1 ) {
				cout << *it1 << " ";
			}
			cout << endl;
		}
		cout << "---------------------------------------------------" << endl;
#endif
		return;
	}
	if ( minResult == 1 ) return;
	map< string, state>::iterator p = pState; //temp pointer for ++pState
	int back = 0; //position of *pState intempSet
	for ( ; back < stateSet.size(); ++back ) //if pState had been inserted into stateSet, back will locate to true pos
		if ( stateSet[back].find((*pState).first) != stateSet[back].end()) break;

	if ( back == stateSet.size() ) { 
		stateSet.push_back(set<string>());
		stateSet[back].insert((*pState).first);
		position[(*pState).first] = back;
	}
	map< string, state>::iterator next = pState;
	++next;
	vector<set<string>> tempSet;
	map<string, int> pos;

	// merge with same states
	for ( set< string >::iterator it = (*pState).second.same.begin(); it != (*pState).second.same.end(); ++it ) {
		if ( *it < (*pState).first ) continue;
		tempSet = stateSet;
		pos = position;
		singleMerge(tempSet, (*pState).first, *it, pos);
		if ( tempSet.empty() ) continue;
		merge(tempSet, next, pos);
	}
	pos = position;
	tempSet = stateSet;
	merge(tempSet, next, pos); // no merge with other states
}

void singleMerge(vector<set<string>>& stateSet, string l, string r, map<string,int>& position)
{
	if ( position.find(l) == position.end() ) {
		stateSet.push_back(set<string>());
		stateSet.back().insert(l);
		position[l] = stateSet.size() - 1;
	}

	int posl = position[l];

	if ( position.find(r) == position.end() ) {
		stateSet.push_back(set<string>());
		stateSet.back().insert(r);
		position[r] = stateSet.size() - 1;
	}

	int posr = position[r];
	

	if ( posl == posr )
		return;
	
	set<pair<string, string>> checkList;

	for ( set<string>::iterator itl = stateSet[posl].begin(); itl != stateSet[posl].end(); ++itl ) { 
		for ( set<string>::iterator itr = stateSet[posr].begin(); itr != stateSet[posr].end(); ++itr ) {
			if ( stateMap[*itl].same.find(*itr) == stateMap[*itl].same.end() ) {
				stateSet.clear();
				return;
			}
			
			if ( *itl < *itr ) {
				if ( root.find(pair<string, string>(*itl, *itr)) != root.end() )
					checkList.insert(root[pair<string,string>(*itl, *itr)].begin(), root[pair<string, string>(*itl, *itr)].end());
			}
			else if ( *itl > *itr ) {
				if ( root.find(pair<string, string>(*itr, *itl)) != root.end() )
					checkList.insert(root[pair<string, string>(*itr, *itl)].begin(), root[pair<string, string>(*itr, *itl)].end());
			}
			
		}
	}

	int pos;

	bool isLgreaterR = posl > posr;
	if ( isLgreaterR ) {
		pos = posl;
		posl = posr;
		posr = pos;
	}
	

	for ( set<string>::iterator it = stateSet[posr].begin(); it != stateSet[posr].end(); ++it )
		position[*it] = posl;
	pos = stateSet.size();
	for ( vector<set<string>>::iterator it = stateSet.end();; --it, --pos ) { // l <-- r
		if ( pos == posr ) {
			stateSet[posl].insert(stateSet[posr].begin(), stateSet[posr].end());
			stateSet.erase(it);

			for ( map<string, int>::iterator p = position.begin(); p != position.end(); ++p )
				if ( (*p).second > pos )
					--(*p).second;

			break;
		}
	}

	for ( set<pair<string, string>>::iterator it = checkList.begin(); it != checkList.end(); ++it ) {
		singleMerge(stateSet, (*it).first, (*it).second, position);
		if ( stateSet.empty() ) return;
	}
	
}

bool isEqual(const vector<set<string>>& l, const vector<set<string>>& r)
{
	set<int> rFind;
	for ( int i = 0; i < l.size(); ++i ) {
		bool e = false;

		for ( int j = 0; j < r.size(); ++j ) {
			if ( rFind.find(j) != rFind.end() ) continue;

			if ( l[i] == r[j] ) {
				e = true;
				rFind.insert(j);
				break;
			}
		}

		if ( !e )
			return false;
	}
	return true;
}

bool cmpState(string l, string r)
{
#if	SHOW_CMP == 1
	cout << "( " << l << " , " << r << " )" << endl;
#endif
	if ( banedRoot.find(pair<string, string>(l, r)) != banedRoot.end() )  //already check false (l, r)		
		return false;
	
	if ( root.find(pair<string, string>(l, r) ) != root.end() ) 
		return true;
	
	//cmp two state's all input
	for ( nextState::iterator it1 = stateMap[l].list.begin(); it1 != stateMap[l].list.end(); ++it1 ) { 
		for ( nextState::iterator it2 = stateMap[r].list.begin(); it2 != stateMap[r].list.end(); ++it2 ) {
			string str1 = (*it1).second.first, str2 = (*it2).second.first;
			
			if ( cmpCode((*it1).first, (*it2).first) ) { //the same input
				if ( (*it1).second.first > (*it2).second.first ) {
					str1 = (*it2).second.first;
					str2 = (*it1).second.first;
				}
				if ( banedRoot.find(pair<string, string>(str1, str2)) != banedRoot.end() ) {
#if	SHOW_CMP == 1
					cout << "( " << str1 << " , " << str2 << " )" << endl;
#endif
					return false; //already check false (it1, it2)
				}
				if ( str1 != str2 )
					root[pair<string, string>(l, r)].insert(pair<string, string>(str1, str2));
				if ( root.find(pair<string, string>(str1, str2)) != root.end() ) {
					//cout << "( " << str1 << " , " << str2 << " )" << endl;
					continue;
				}

				//check output
				if ( !cmpCode((*it1).second.second, (*it2).second.second) ) { //have different output
#if	SHOW_CMP == 1
					cout << endl << (*it1).first << " " << l << " " << (*it1).second.first << " "
						<< (*it1).second.second
						<< endl << (*it2).first << " " << r << " " << (*it2).second.first << " "
						<< (*it2).second.second << endl
						<< "( " << l << " , " << r << " )"
						<< " have different <output> : "
						<< (*it1).second.second << " " << (*it2).second.second << endl;
#endif
					banedRoot.insert(pair<string, string>(l, r));
					return false;
				}

				//check next-state
				if ( (*it1).second.first == l && (*it2).second.first == r ); //each next-state are self state 
				if ( (*it1).second.first != (*it2).second.first ) { //have different state
					
					if ( stateMap[(*it1).second.first].same.find((*it2).second.first) 
						== stateMap[(*it1).second.first].same.end() ) { //if it1 != it2 (next-state) and haven't cmp
#if	SHOW_CMP == 1	
						cout << endl << (*it1).first << " " << l << " " << (*it1).second.first << " "
							<< (*it1).second.second
							<< endl << (*it2).first << " " << r << " " << (*it2).second.first << " "
							<< (*it2).second.second << endl
							<< "( " << l << " , " << r << " )"
							<< " have different <next state> : "
							<< (*it1).second.first << " " << (*it2).second.first << endl;
#endif
						if ( !cmpState(str1, str2) ) { //recursive call, cmp next state
							banedRoot.insert(pair<string, string>(str1, str2));
							return false;
						}
					}
				}
			}
		}
	}
	return true;
}

bool cmpCode(string a, string b)
{
	for ( int i = 0; i < a.size(); ++i ) {
		if ( a[i] != b[i] )
			if ( a[i] != '-' && b[i] != '-' )
				return false;
	}
	return true;
}

void finishMap()
{
#if SHOW_MERGEINPUT == 1
	cout << "finishMap() begin" << endl;
#endif
	list< vector< set<string> > >::iterator it_list = listResult.begin();
	
	for ( int outputIndex = 0; outputIndex < MAX_COUNT && outputIndex < listResult.size(); ++outputIndex, ++it_list ) { 
		fullstateMap.push_back(stateMap);
		vector<string> minList((*it_list).size(), "");
		int Terms = 0;
		/*
		i = 0 , it = (*it_list).begin()
		1 2 3 <-- *it = 1
		4 5 6
		*/

		for ( int i = 0; i < (*it_list).size(); ++i ) { //pos of a output (vector[i])
			set<string>::iterator it = (*it_list)[i].begin(); //iterator of set

			string min = (*it);
			if ( (*it_list)[i].find(resetState) != (*it_list)[i].end() ) min = resetState;

			for ( ; it != (*it_list)[i].end(); ++it ) {
				if ( *it == min ) continue;
				
				// let all next-state of *it insert into min 
				fullstateMap.back()[min].list.insert(fullstateMap.back()[*it].list.begin(),
													 fullstateMap.back()[*it].list.end());
				fullstateMap.back().erase(*it);
			}
			minList[i] = min;
		}

		
		//change next-state
		for ( map<string, state>::iterator it = fullstateMap.back().begin(); it != fullstateMap.back().end(); ++it ) {
			Terms += (*it).second.list.size();
			for ( nextState::iterator it1 = (*it).second.list.begin();
				 it1 != (*it).second.list.end(); ++it1 ) {
				for ( int i = 0; i < (*it_list).size(); ++i ) {
					if ( (*it_list)[i].find((*it1).second.first) != (*it_list)[i].end() ) {
						(*it1).second.first = minList[i];
						break;
					}
				}
			}
		}
		
		if ( minTerm > Terms ) {
			minTerm = Terms;
			minTerm_pos = outputIndex;
		}
	}
}

void output()
{
	ofstream outFile("output.kiss", ios::out);
	outFile << ".i" << " " << numInputs << endl
		<< ".o" << " " << numOutputs << endl
		<< ".p" << " " << minTerm << endl
		<< ".s" << " " << fullstateMap[minTerm_pos].size() << endl
		<< ".r" << " " << resetState << endl;

	for ( map<string, state>::iterator it = fullstateMap[minTerm_pos].begin(); it != fullstateMap[minTerm_pos].end(); ++it ) {
		for ( nextState::iterator it2 = (*it).second.list.begin(); it2 != (*it).second.list.end(); ++it2 ) {
			outFile << (*it2).first << " "
				<< (*it).first << " "
				<< (*it2).second.first << " "
				<< (*it2).second.second << endl;
		}
	}
	outFile << ".e" << endl;
	cout << "\nFinished!\n" << endl;
	outFile.close();
}

void showResult()
{

	cout << ".i" << " " << numInputs << endl
		<< ".o" << " " << numOutputs << endl
		<< ".p" << " " << minTerm << endl
		<< ".s" << " " << fullstateMap[minTerm_pos].size() << endl
		<< ".r" << " " << resetState << endl;

	for ( map<string, state>::iterator it = fullstateMap[minTerm_pos].begin(); it != fullstateMap[minTerm_pos].end(); ++it ) {
		for ( nextState::iterator it2 = (*it).second.list.begin(); it2 != (*it).second.list.end(); ++it2 ) {
			cout << (*it2).first << " "
				<< (*it).first << " "
				<< (*it2).second.first << " "
				<< (*it2).second.second << endl;
		}
	}
	cout << ".e";
}
void test()
{
	cout << "start test" << endl;
	string currentStateA, currentStateB;
	string outputA, outputB;
	string input;
	nextState::iterator it;
	nextState::iterator it2;
	for ( int k = 0; k < fullstateMap.size(); ++k ) {
		cout << k << endl;
		for ( int seed = 1; seed < SEED; ++seed ) {
			//cout << seed << endl;
			srand(seed);
			currentStateA = currentStateB = resetState;
			for ( int testcase = 0, i = 0, Size; testcase < TESTCASE; ++testcase, i = 0 ) {
				if ( stateMap[currentStateA].list.size() == 0 )
					break;

				Size = rand() % stateMap[currentStateA].list.size();
				it = stateMap[currentStateA].list.begin();
				for ( ; i != Size; ++i, ++it );
				input = (*it).first;

				for ( int i = 0; i < input.size(); ++i ) {
					if ( input[i] == '-' )
						input[i] = '0' + (rand() % 2);
				}
				
				outputA = (*it).second.second;

				for ( it2 = fullstateMap[k][currentStateB].list.begin(); it2 != fullstateMap[k][currentStateB].list.end(); ++it2 ) {
					if ( cmpCode(input, (*it2).first) ) {
						outputB = (*it2).second.second;
						
						break;
					}
				}
				if ( it2 == fullstateMap[k][currentStateB].list.end() ) {
					system("pause");
				}

				if ( outputA != outputB ) {
					cout << seed << " " << testcase << endl;
					cout << "input :" << input << endl;
					cout << "current : " << currentStateA << " " << currentStateB << endl;
					cout << "output  : " << outputA << " " << outputB << endl << endl;
					break;
				}
				currentStateA = (*it).second.first;
				currentStateB = (*it2).second.first;
			}
		}
	}
	cout << "Finished!" << endl;
}
