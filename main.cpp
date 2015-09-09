#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>

#include <cstring>

#include <time.h> //Take time

#include <unistd.h> //usleep
#include <math.h> //for Word2Vec
#include <malloc.h> //for Word2Vec

#include "internetQueryFunctions.cpp"
//#include "internetQueryToDBPedia.cpp"

using namespace std;


struct roomOrderStruct {
  int ID;
  double probability;
};

bool wayToSortRoomOrderProbability(roomOrderStruct i, roomOrderStruct j) {
    return i.probability > j.probability;
}

void sortWithRandom(vector<roomOrderStruct> &roomOrder) {
  for (int i=0; i<roomOrder.size()-1; i++) {
    for (int j=i+1; j<roomOrder.size(); j++) {
      if (roomOrder[i].probability < roomOrder[j].probability) {
	roomOrderStruct tmp=roomOrder[i];
	roomOrder[i]=roomOrder[j];
	roomOrder[j]=tmp;
      }
      else if ((roomOrder[i].probability == roomOrder[j].probability) && (rand() % 2 == 1)) { //Having equal probability, change order randomly for not getting always 0,1,2...
	roomOrderStruct tmp=roomOrder[i];
	roomOrder[i]=roomOrder[j];
	roomOrder[j]=tmp;
      }
    }
  }
}

void orderByIndex(vector <int> &a) {
  vector <int> b=a;
  for (int i=0;i<a.size();i++) {
    b[ a[i] ]=i;
  }
  a=b;
}

string printVector(vector <int> a) {
  string s="";
  for (int i=0;i<a.size();i++) {
    ostringstream n;
    n << a[i]; //to_string(5);
    s = s + string(n.str()) + ",";
  }
  if (a.size()>0) s.erase(s.size()-1,1);
  return s;
}

string printVector(vector <double> a) {
  string s="";
  double t=0;
  for (int i=0;i<a.size();i++) {
    ostringstream n;
    n << a[i];
    s = s + string(n.str()) + ",";
    t+=a[i];
  }
  if (a.size()>0) s.erase(s.size()-1,1);
  /* Print sum of probabilities to check normalization to 1
  ostringstream n;
  n << t;
  s = s + "          " + string(n.str());
  */
  return s;
}

void normalizeVector(vector <double> &a) {
  double t=0;
  for (int i=0;i<a.size();i++) {
    t+=a[i];
  }
  for (int i=0;i<a.size();i++) {
    if (t!=0) a[i]/=t;
    else a[i] = 0;
  }
  return;
}


//invented by me
double NOkendallTauNormalized(vector <int> order1, vector <int> order2) {
  if ( order1.empty() || order2.empty() ) {
    return 1; //Order lists not must be empty
    //return 0; //unless don't want to count empty lists
  }
  if ( order1.size()!=order2.size() ) {
    return 1; //Order lists must be same size
  }
  const double factorIncrement=10;
  double factor=factorIncrement;
  double sumValues=0;
  double maxValue=0;
  for (int i=order1.size()-2;i>=0;i--) {
    for (int j=i; j<order2.size();j++) {
      if ( order1[i]==order2[j] ) {
	sumValues+=(j-i)*factor;
	break;
	}
    }
    int maxDistance=(order1.size()+1-i*2);
    if (maxDistance>0) maxValue+=maxDistance*factor;
    factor*=factorIncrement;
  }
  //cout << "Sum " << sumOppositeOrder << endl;
  return (sumValues/maxValue );
}


//weightedKendallTau found in "Generalized Distances between Rankings" but not a Normalized version
//TODO Research questions: what is the max value of a weighted Kendall? Is always the inverse vector?
//TODO Think in weighs not monotonal decreasing like the humans in room order: high firsts and lasts weighs.
double weightedKendallTauNormalized(vector <int> order1, vector <int> order2) {
  static const double weightsTMP[] = {0.2109575164, 0.1561700081, 0.098310315, 0.073732727, 0.0640040992, 0.0547874715, 0.0609319146, 0.0711725793, 0.0762928379, 0.1336405309}; //From matches percentage of users
  //static const double weightsTMP[] = {0.2096410822, 0.1716396011, 0.1405266198, 0.1150534653, 0.0941978103, 0.0771226441, 0.0631426805, 0.0516968544, 0.0423258045, 0.0346534378}; //From exponential function with lambda 0.4
  vector<double> weights (weightsTMP, weightsTMP + sizeof(weightsTMP) / sizeof(weightsTMP[0]) );
  //double maxValue = 0.00965703; //For normalizing
  
  if ( order1.empty() || order2.empty() ) {
    return 1; //Order lists not must be empty
    //return 0; //unless don't want to count empty lists
  }
  if ( order1.size()!=order2.size() ) {
    return 1; //Order lists must be same size
  }
  
  double maxValue = 0; //Getting the maxValue of the distance for normalizing (vector inverted)
  vector <int> tmpOrder1, tmpOrder2;
  for (int i=0; i<order1.size();i++) {
    tmpOrder1.push_back(i+1);
    tmpOrder2.push_back(order1.size()-i);
  }
  for (int i=0; i<tmpOrder1.size();i++) {
    for (int j=i+1; j<tmpOrder2.size();j++) {
      if ( ( tmpOrder1[i]>tmpOrder1[j] && tmpOrder2[i]<tmpOrder2[j] ) ||
	( tmpOrder1[i]<tmpOrder1[j] && tmpOrder2[i]>tmpOrder2[j] ) ) {
	maxValue += weights[i]*weights[j];
	}
    }
  }
  
  double sumOppositeOrder=0;
  for (int i=0; i<order1.size();i++) {
    for (int j=i+1; j<order2.size();j++) {
      if ( ( order1[i]>order1[j] && order2[i]<order2[j] ) ||
	( order1[i]<order1[j] && order2[i]>order2[j] ) ) {
	sumOppositeOrder += weights[i]*weights[j];
	}
    }
  }
  return (sumOppositeOrder/maxValue);
}

//kendallTauNormalized
double kendallTauNormalized(vector <int> order1, vector <int> order2) {
  if ( order1.empty() || order2.empty() ) {
    return 1; //Order lists not must be empty
    //return 0; //unless don't want to count empty lists
  }
  if ( order1.size()!=order2.size() ) {
    return 1; //Order lists must be same size
  }
  int sumOppositeOrder=0;
  for (int i=0; i<order1.size();i++) {
    for (int j=i+1; j<order2.size();j++) {
      if ( ( order1[i]>order1[j] && order2[i]<order2[j] ) ||
	   ( order1[i]<order1[j] && order2[i]>order2[j] ) ) {
	sumOppositeOrder++;
      }
    }
  }
  //cout << "Sum " << sumOppositeOrder << endl;
  return (sumOppositeOrder/( order1.size()*(double)(order1.size()-1) / 2.0) );
}


vector <int> extractOrder(string str, int maxRoom) {
  //string str must follow format objectNumber-orderRoom1,orderRoom2,orderRoom3,etc
  vector <int> A;
  size_t separatorPos = str.find('-',0);
  while (separatorPos!=string::npos && separatorPos<str.length()) {
    size_t separatorPos2 = str.find(',',separatorPos+1);
    if (separatorPos2==string::npos) separatorPos2=str.length();
    string numberSTR=str.substr(separatorPos+1,separatorPos2-(separatorPos+1));
    int number = atoi(numberSTR.c_str());
    if (number < maxRoom ) A.push_back( number );
    separatorPos=separatorPos2;
  }
  return A;
}


vector <double> extractOrderProbability(string str) {
  //string str must follow format objectName,probabilityRoom1,probabilityRoom2,probabilityRoom3,etc
  vector <double> A;
  size_t separatorPos = str.find(',',0);
  while (separatorPos!=string::npos && separatorPos<str.length()) {
    size_t separatorPos2 = str.find(',',separatorPos+1);
    if (separatorPos2==string::npos) separatorPos2=str.length();
    string numberSTR=str.substr(separatorPos+1,separatorPos2-(separatorPos+1));
    double number = atof(numberSTR.c_str());
    A.push_back( number );
    separatorPos=separatorPos2;
  }
  return A;
}


void orderFromGoogle(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {
  
  vector <long int> roomsHits;
  long int sumRoomsHits=0;
  vector <long int> roomsAndObjectHits;
  long int sumRoomsAndObjectHits=0;
  //vector <double> roomsAndObjectProbability;
  for (int i=0; i<rooms.size(); i++) {
    roomsHits.push_back( googleHitsCount(rooms[i]) );
    sumRoomsHits+=roomsHits[i];
    roomsAndObjectHits.push_back( googleHitsCount(object + "+" + rooms[i]) ); 
    sumRoomsAndObjectHits+=roomsAndObjectHits[i];
    usleep(5000000);
  }
  
  //My distance method
  /*
  for (int i=0; i<rooms.size(); i++) {
    double noStandarProbability = ( roomsAndObjectHits[i] * sumRoomsHits ) / (double) ( roomsHits[i] * sumRoomsAndObjectHits );
    roomsAndObjectProbability.push_back( noStandarProbability );
  }
  */
 
 //Normalized Google Distance =  [  max ( log object,  log room) - log object+room  ] / [  log M - min ( log object,  log room) ]
 long int objectHits=googleHitsCount(object);
 long int M=60000000000;
  for (int i=0; i<rooms.size(); i++) {
    double noStandarProbability = ( max( log(objectHits), log(roomsHits[i]) ) - log(roomsAndObjectHits[i]) ) / ( log(M) - min( log(objectHits), log(roomsHits[i]) )  );
    roomsAndObjectProbability.push_back( noStandarProbability );
  }
  
  normalizeVector(roomsAndObjectProbability);
  
  //The distance is closer with more probability. So invert it with 1-Distance
  for (int i=0; i<rooms.size(); i++) {
    roomsAndObjectProbability[i] = 1 - roomsAndObjectProbability[i];
  }
  
  
  /*
  //------------------------------------------------------------------------------------------------------------------
  // Show internet query statistics
  cout << "Google Order for object: " << object << endl;   
  for (int i=0; i<rooms.size(); i++) {
    cout << "Room: " << rooms[i] << "   Room Hits: "<< roomsHits[i] << "   Hits: "<< roomsAndObjectHits[i] << "   Probability: " << roomsAndObjectProbability[i] << endl ;
  }
  */
  
  vector<roomOrderStruct> roomOrder;
  for (int i=0; i<roomsAndObjectProbability.size(); i++) {
    roomOrderStruct roomNode;
    roomNode.ID=i;
    if ( isnan(roomsAndObjectProbability[i]) ) roomsAndObjectProbability[i] = 0;
    roomNode.probability = roomsAndObjectProbability[i];
    roomOrder.push_back(roomNode);
  }
  sortWithRandom(roomOrder); //===============================================================
  //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
  for (int i=0; i<roomOrder.size(); i++) {
    order.push_back(roomOrder[i].ID);
  }
  orderByIndex(order);
}



void orderFromDBPedia(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {

  vector <long int> roomsHits;
  long int sumRoomsHits=0;
  vector <long int> roomsAndObjectHits;
  long int sumRoomsAndObjectHits=0;
  //vector <double> roomsAndObjectProbability;
  for (int i=0; i<rooms.size(); i++) {
    roomsHits.push_back( dbpediaHitsCount(rooms[i]) );
    sumRoomsHits+=roomsHits[i];
    roomsAndObjectHits.push_back( dbpediaHitsCount(object + " " + rooms[i]) ); 
    sumRoomsAndObjectHits+=roomsAndObjectHits[i];
  }
  
  for (int i=0; i<rooms.size(); i++) {
    double noStandarProbability = ( roomsAndObjectHits[i] * sumRoomsHits ) / (double) ( roomsHits[i] * sumRoomsAndObjectHits );
    roomsAndObjectProbability.push_back( noStandarProbability );
  }
  normalizeVector(roomsAndObjectProbability);
  /*
  // Show internet query statistics
  cout << "DBPedia Order for object: " << object << endl;   
  for (int i=0; i<rooms.size(); i++) {
    cout << "Room: " << rooms[i] << "   Room Hits: "<< roomsHits[i] << "   Hits: "<< roomsAndObjectHits[i] << "   Probability: " << roomsAndObjectProbability[i] << endl ;
  }
  */
  vector<roomOrderStruct> roomOrder;
  for (int i=0; i<roomsAndObjectProbability.size(); i++) {
    roomOrderStruct roomNode;
    roomNode.ID=i;
    if ( isnan(roomsAndObjectProbability[i]) ) roomsAndObjectProbability[i] = 0;
    roomNode.probability=roomsAndObjectProbability[i];
    roomOrder.push_back(roomNode);
  }
  sortWithRandom(roomOrder); //===============================================================
  //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
  for (int i=0; i<roomOrder.size(); i++) {
    order.push_back(roomOrder[i].ID);
  }
  orderByIndex(order);
}


void orderFromConceptNet5(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {
  
  vector <long int> roomsAndObjectHits;
  long int sumRoomsAndObjectHits=0;
  //vector <double> roomsAndObjectProbability;
  for (int i=0; i<rooms.size(); i++) {
    roomsAndObjectHits.push_back( conceptNet5HitsCount(object + " " + rooms[i]) ); 
    sumRoomsAndObjectHits+=roomsAndObjectHits[i];
  }
  
  for (int i=0; i<rooms.size(); i++) {
    roomsAndObjectProbability.push_back( roomsAndObjectHits[i]/( (double) sumRoomsAndObjectHits ) );
  }
  /*
  // Show internet query statistics
  cout << "ConceptNet5 Order for object: " << object << endl;   
  for (int i=0; i<rooms.size(); i++) {
    cout << "Room: " << rooms[i] << "   Hits: "<< roomsAndObjectHits[i] << "   Probability: " << roomsAndObjectProbability[i] << endl ;
  }
  */
  vector<roomOrderStruct> roomOrder;
  for (int i=0; i<roomsAndObjectProbability.size(); i++) {
    roomOrderStruct roomNode;
    roomNode.ID=i;
    if ( isnan(roomsAndObjectProbability[i]) ) roomsAndObjectProbability[i] = 0;
    roomNode.probability=roomsAndObjectProbability[i];
    roomOrder.push_back(roomNode);
  }
  sortWithRandom(roomOrder); //===============================================================
  //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
  for (int i=0; i<roomOrder.size(); i++) {
    order.push_back(roomOrder[i].ID);
  }
  orderByIndex(order);
}




void orderFromUser(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {
  cout << "Rooms in map:" << endl;
  for (int i=0; i<rooms.size(); i++) {
    cout << i << "-> " << rooms[i] << endl;
  }
  cout << "Enter order for searching object " << object << " separated by enter:" << endl;
  
  for (int i=0; i<rooms.size(); i++) {
    int number;
    cin >> number;
    order.push_back(number);
    roomsAndObjectProbability.push_back(1); //Create vector with rooms size
  }
  for (int i=0; i<rooms.size(); i++) {
    roomsAndObjectProbability[ order[i] ]= 1/( (double)i+1) ;
  }
  normalizeVector(roomsAndObjectProbability);
  orderByIndex(order);
}

void orderFromRandom(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {
  for (int i=0; i<rooms.size(); i++) {
    roomsAndObjectProbability.push_back( rand()/( (double)RAND_MAX) );
  }
  normalizeVector(roomsAndObjectProbability);
  vector<roomOrderStruct> roomOrder;
  for (int i=0; i<roomsAndObjectProbability.size(); i++) {
    roomOrderStruct roomNode;
    roomNode.ID=i;
    roomNode.probability=roomsAndObjectProbability[i];
    roomOrder.push_back(roomNode);
  }
  sortWithRandom(roomOrder); //===============================================================
  //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
  for (int i=0; i<roomOrder.size(); i++) {
    order.push_back(roomOrder[i].ID);
  }
  orderByIndex(order);
}

void orderFromFixed(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {
  for (int i=0; i<rooms.size(); i++) {
    roomsAndObjectProbability.push_back( 1/( (double)i+1) );
    order.push_back(i);
  }
  normalizeVector(roomsAndObjectProbability);
  orderByIndex(order);
}


bool dictionaryLoaded=false;
float *M;
char *vocab;


void orderFromWord2Vec(vector<string> rooms, vector <int> &order, vector <double> &roomsAndObjectProbability, string object) {
  const long long max_size = 2000;         // max length of strings
  const long long N = 10;                  // number of closest words that will be shown
  const long long max_w = 50;              // max length of vocabulary entries
  
  FILE *f;
  char st1[max_size];
  char *bestw[N];
  char file_name[max_size], st[100][max_size];
  int stgroup[100];
  float dist, len, bestd[N], vec[max_size], vecnormalized[N][max_size];
  long long words, size, a, b, c, d, g, cn, bi[100],cgroup;
  char ch;
  
  //float *M;
  //char *vocab;
  
  if (!dictionaryLoaded) {
  strcpy(file_name, "/host/Users/IzquierdoCR/Desktop/Maestria INAOE/Sem 4 - Tesis/word2vec/GoogleNews-vectors-negative300.bin");
  f = fopen(file_name, "rb");
  if (f == NULL) {
    cout << "Input file not found" << endl;
    return;
  }
  fscanf(f, "%lld", &words);
  fscanf(f, "%lld", &size);
  vocab = (char *)malloc((long long)words * max_w * sizeof(char));
  for (a = 0; a < N; a++) bestw[a] = (char *)malloc(max_size * sizeof(char));
  M = (float *)malloc((long long)words * (long long)size * sizeof(float));
  if (M == NULL) {
    
    cout << "Cannot allocate memory: " << (long long)words * size * sizeof(float) / 1048576 <<" MB    " << words << "  " << size << endl;
    return;
  }
  for (b = 0; b < words; b++) {
    a = 0;
    while (1) {
      vocab[b * max_w + a] = fgetc(f);
      if (feof(f) || (vocab[b * max_w + a] == ' ')) break;
      if ((a < max_w) && (vocab[b * max_w + a] != '\n')) a++;
    }
    vocab[b * max_w + a] = 0;
    for (a = 0; a < size; a++) fread(&M[a + b * size], sizeof(float), 1, f);
    len = 0;
    for (a = 0; a < size; a++) len += M[a + b * size] * M[a + b * size];
    len = sqrt(len);
    for (a = 0; a < size; a++) M[a + b * size] /= len;
  }
  fclose(f);
  
  dictionaryLoaded=true;
  }
  
  //cout << "Dictionary load complete" << endl;
  for (a = 0; a < N; a++) bestd[a] = 0;
  for (a = 0; a < N; a++) bestw[a][0] = 0;
  
  /*
   * USAGE: orderFromWord2Vec("apple,kitchen,bathroom,etc")
   * Extension: object word1relatedtoobject word2relatedtoobject etc,room1 word1relatedtoroom1  word2relatedtoroom1 etc,
   * room2 word1relatedtoroom2  word2relatedtoroom2 etc,room3 ...
   */
  string st1String=object+",";
  for (int a=0; a<rooms.size(); a++) {
    st1String=st1String+rooms[a]+",";
  }
  
  st1String.erase(st1String.length()-1);
  st1String=st1String+"\0";
  strcpy(st1,st1String.c_str());
  
  cn = 0;
  b = 0;
  c = 0;
  cgroup = 0; //Cuantos grupos de palabras hay
  while (1) {
    st[cn][b] = st1[c];
    b++;
    c++;
    st[cn][b] = 0;
    stgroup[cn] = cgroup; //Se repite con cada caracter pero se programa rapido
    if (st1[c] == 0) break;
    if (st1[c] == ' ') {
      cn++;
      b = 0;
      c++;
    }
    if (st1[c] == ',') { //Es otro grupo
      cn++;
      b = 0;
      c++;
      cgroup++;
    }
  }
  //Agregar que imprima el tamaño de la BD
  //cout << "Database: "<<words<< " Words in vocabulary and vectors in R"<<size << endl;
  cn++;
  for (a = 0; a < cn; a++) {
    for (b = 0; b < words; b++) if (!strcmp(&vocab[b * max_w], st[a])) break;
    if (b == words) b = -1;
    bi[a] = b;
    //cout << "Word: "<< st[a] <<" in Group " << stgroup[a] << ". Position in vocabulary: " << bi[a] << endl;
    if (b == -1) {
      cout << "Out of dictionary word!" << endl;
      break;
    }
  }
  
  //cout << "                                              Group      Cosine distance" << endl;
  //cout << "------------------------------------------------------------------------" << endl;
  
  for (g = 0; g <= cgroup; g++) { //Sumar los vectores de los diferentes grupos
    
    for (a = 0; a < size; a++) vec[a] = 0;
    for (b = 0; b < cn; b++) {
      if (bi[b] == -1) continue;
      if (stgroup[b] == g) for (a = 0; a < size; a++) vec[a] += M[a + bi[b] * size]; //Solo suma los del grupo
    }
    //Normaliza
    len = 0;
    for (a = 0; a < size; a++) len += vec[a] * vec[a];
    len = sqrt(len);
    for (a = 0; a < size; a++) {
      vec[a] /= len;
      //Guarda en la lista de vectores normalizados
      vecnormalized[g][a]=vec[a];
    }
  }
  
  //Hacemos N=10 para no buscar tantas palabras e inicializa la lista
  for (a = 0; a < N; a++) bestd[a] = -1;
  for (a = 0; a < N; a++) bestw[a][0] = 0;
  
  //vector <double> roomsAndObjectProbability;
  
  //Busca por las palabras de la lista que dieron menos la primera que es referencia
  for (c = 1; c <= cgroup; c++) {
    dist = 0;
    for (a = 0; a < size; a++) dist += vecnormalized[0][a] * vecnormalized[c][a];
    dist=fabs(dist);
    //cout << "                                                 " << c << "\t\t" << dist << endl;
    roomsAndObjectProbability.push_back(dist);
  }
  
  //Free memory used by dictionary
  //free(M);
  //free(vocab);
  
  
  vector<roomOrderStruct> roomOrder;
  for (int i=0; i<roomsAndObjectProbability.size(); i++) {
    roomOrderStruct roomNode;
    roomNode.ID=i;
    if ( isnan(roomsAndObjectProbability[i]) ) roomsAndObjectProbability[i] = 0;
    roomNode.probability=roomsAndObjectProbability[i];
    roomOrder.push_back(roomNode);
  }
  sortWithRandom(roomOrder); //===============================================================
  //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
  for (int i=0; i<roomOrder.size(); i++) {
    order.push_back(roomOrder[i].ID);
  }
  normalizeVector(roomsAndObjectProbability);
  orderByIndex(order);
  
  
  return ;

}

//This function generates Weighted Probability order once the files containing others methods are already generated
void orderFromWeightedProbabilitiy(vector<string> rooms, vector<string> objects, vector<string> votersFiles, string outputFile, vector<double> weights) {
  
  ofstream outputProbability( outputFile.c_str() );
  outputFile.insert(outputFile.find("."),"Pb");
  ofstream outputProbabilityPb( outputFile.c_str() );
  ifstream inputProbability[ votersFiles.size() ];
  for (int j=0; j<votersFiles.size(); j++) {
    votersFiles[j].insert(votersFiles[j].find("."),"Pb");
    inputProbability[j].open( votersFiles[j].c_str() );
  }
  for (int i=0; i<objects.size(); i++) {
    vector<double> accumulatedVotes( rooms.size(), 0.0);
    for (int j=0; j<votersFiles.size(); j++) {
      vector <double> voter;
      string line;
      getline(inputProbability[j], line); //This do not check if the file is incomplete or unstructured
      voter=extractOrderProbability( line );
      
      for (int k=0; k<rooms.size(); k++) {
	accumulatedVotes[k]+=voter[k]*weights[j];
      }
    }
    
    normalizeVector(accumulatedVotes);
    vector<roomOrderStruct> roomOrder;
    for (int j=0; j<accumulatedVotes.size(); j++) {
      roomOrderStruct roomNode;
      roomNode.ID=j;
      roomNode.probability=accumulatedVotes[j];
      roomOrder.push_back(roomNode);
    }
    sortWithRandom(roomOrder); //===============================================================
    //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
    vector <int> orderProbability;
    for (int j=0; j<roomOrder.size(); j++) {
      orderProbability.push_back(roomOrder[j].ID);
    }
    normalizeVector(accumulatedVotes);
    outputProbability << i << "-" << printVector(orderProbability) << endl;
    outputProbabilityPb << objects[i] << "," << printVector(accumulatedVotes) << endl;
  }
  outputProbability.close();
  outputProbabilityPb.close();
  for (int j=0; j<votersFiles.size(); j++) {
    inputProbability[j].close();
  }
}


//This function generates Probability order once the files containing others methods are already generated
void orderFromProbabilitiy(vector<string> rooms, vector<string> objects, vector<string> votersFiles, string outputFile) {
  ofstream outputProbability( outputFile.c_str() );
  outputFile.insert(outputFile.find("."),"Pb");
  ofstream outputProbabilityPb( outputFile.c_str() );
  ifstream inputProbability[ votersFiles.size() ];
  for (int j=0; j<votersFiles.size(); j++) {
    votersFiles[j].insert(votersFiles[j].find("."),"Pb");
    inputProbability[j].open( votersFiles[j].c_str() );
  }
  for (int i=0; i<objects.size(); i++) {
    vector<double> accumulatedVotes( rooms.size(), 0.0);
    for (int j=0; j<votersFiles.size(); j++) {
      vector <double> voter;
      string line;
      getline(inputProbability[j], line); //This do not check if the file is incomplete or unstructured
      voter=extractOrderProbability( line );
      
      for (int k=0; k<rooms.size(); k++) {
	  accumulatedVotes[k]+=voter[k];
      }
    }
    
    normalizeVector(accumulatedVotes);
    vector<roomOrderStruct> roomOrder;
    for (int j=0; j<accumulatedVotes.size(); j++) {
      roomOrderStruct roomNode;
      roomNode.ID=j;
      roomNode.probability=accumulatedVotes[j];
      roomOrder.push_back(roomNode);
    }
    sortWithRandom(roomOrder); //===============================================================
    //sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
    vector <int> orderProbability;
    for (int j=0; j<roomOrder.size(); j++) {
      orderProbability.push_back(roomOrder[j].ID);
    }
    normalizeVector(accumulatedVotes);
    outputProbability << i << "-" << printVector(orderProbability) << endl;
    outputProbabilityPb << objects[i] << "," << printVector(accumulatedVotes) << endl;
  }
  outputProbability.close();
  outputProbabilityPb.close();
  for (int j=0; j<votersFiles.size(); j++) {
    inputProbability[j].close();
  }
}


//This function generates Borda order once the files containing others methods are already generated
void orderFromBorda(vector<string> rooms, vector<string> objects, vector<string> votersFiles, string outputFile) {
  //static const double weightsTMP[] = {0.2109575164, 0.1561700081, 0.098310315, 0.073732727, 0.0640040992, 0.0547874715, 0.0609319146, 0.0711725793, 0.0762928379, 0.1336405309}; //From matches percentage of users
  static const double weightsTMP[] = {0.2096410822, 0.1716396011, 0.1405266198, 0.1150534653, 0.0941978103, 0.0771226441, 0.0631426805, 0.0516968544, 0.0423258045, 0.0346534378}; //From exponential function with lambda 0.4
  vector<double> weights (weightsTMP, weightsTMP + sizeof(weightsTMP) / sizeof(weightsTMP[0]) );
  
  ofstream outputBorda( outputFile.c_str() );
  outputFile.insert(outputFile.find("."),"Pb");
  ofstream outputBordaPb( outputFile.c_str() );
  ifstream inputBorda[ votersFiles.size() ];
  for (int j=0; j<votersFiles.size(); j++) {
    inputBorda[j].open( votersFiles[j].c_str() );
  }
  for (int i=0; i<objects.size(); i++) {
    vector<double> accumulatedVotes( rooms.size(), 0.0);
    for (int j=0; j<votersFiles.size(); j++) {
      vector <int> voter;
      string line;
      getline(inputBorda[j], line); //This do not check if the file is incomplete or unstructured
      voter=extractOrder( line, rooms.size() );
      for (int k=0; k<rooms.size(); k++) {
	accumulatedVotes[ voter[k] ]+= rooms.size()-(k+1);
      }
    }
    
    vector<roomOrderStruct> roomOrder;
    for (int j=0; j<accumulatedVotes.size(); j++) {
      roomOrderStruct roomNode;
      roomNode.ID=j;
      roomNode.probability=accumulatedVotes[j];
      roomOrder.push_back(roomNode);
    }
    //sortWithRandom(roomOrder); //===============================================================
    sort(roomOrder.begin(), roomOrder.end(), wayToSortRoomOrderProbability);
    vector <int> orderBorda;
    for (int j=0; j<roomOrder.size(); j++) {
      orderBorda.push_back(roomOrder[j].ID);
      
      accumulatedVotes[roomOrder[j].ID]= weights[j]; //Probability by exponential function. Comment for number of votes
    }
    normalizeVector(accumulatedVotes);
    outputBorda << i << "-" << printVector(orderBorda) << endl;
    
    outputBordaPb << objects[i] << "," << printVector(accumulatedVotes) << endl;
  }
  outputBorda.close();
  outputBordaPb.close();
  for (int j=0; j<votersFiles.size(); j++) {
    inputBorda[j].close();
  }
}


void compareRoomMatching(vector<string> rooms, vector<string> objects) {

  int elementsToCompare=17; //The number of users (files) to be analized
  int roomsToEvaluate=10; //Consider only rooms with ID less than this
  int metricToCompareByObject=9; //Compare by object all orders versus BordaUser (Only compare one of all)
  const char *titlesVector[] = {"Google", "DBPedia", "ConceptNet5", "Word2Vec", "Random", "Fixed", "BordaWeb", "ProbabilityWeb", "WeightedProbabilityWeb", "BordaUser", "User1", "User2", "User3", "User4", "User5", "User6", "User7"};
  vector<string> titles(titlesVector, titlesVector+elementsToCompare);
  const char *filesVector[] = {"data/experimentQueryGoogle.txt", "data/experimentQueryDBPedia.txt", "data/experimentQueryConceptNet5.txt", "data/experimentQueryWord2Vec.txt", "data/experimentQueryRandom.txt", "data/experimentQueryFixed.txt", "data/experimentQueryBordaWeb.txt", "data/experimentQueryProbabilityWeb.txt", "data/experimentQueryWeightedProbabilityWeb.txt", "data/bordaUser.txt", "data/users/ramon.txt", "data/users/oac270676.txt", "data/users/palomizee.txt", "data/users/La Torres .txt", "data/users/2077.txt", "data/users/A5FDC06321.txt", "data/users/Ricardo Mastachi.txt"};
  vector<string> files(filesVector, filesVector+elementsToCompare);
  
  
  //Generate combinatories orders for users 
  string bordaUserFile = files[metricToCompareByObject];
  vector<string> votersFilesUsers;
  for (int i=10; i<elementsToCompare ; i++) votersFilesUsers.push_back(files[i]);
  orderFromBorda(rooms, objects, votersFilesUsers, bordaUserFile);
  //orderFromProbability is not possible in users due to users didn't capture probabilities


  
  //start comparing global results
  ofstream outputUsersMatchingRooms("data/experimentsRoomMatching.txt");
  for (int i=0; i<elementsToCompare; i++) { //Write field titles in the results files
    outputUsersMatchingRooms << titles[i];
    if (i<elementsToCompare-1) {
      outputUsersMatchingRooms << ",";
    }
  }
  outputUsersMatchingRooms << endl;
  
  for (int i=0; i<rooms.size(); i++) {   //Para usarlo con Kendall Tau 
  int roomsToEvaluate=10;
  
    for (int j=0; j<elementsToCompare; j++) {
      //double sumKendallTau;
      int sumRoomMatch=0;
      ifstream inputBordaUser(bordaUserFile.c_str());
      ifstream inputFile(files[j].c_str());
      //cout << "File " << files[j] << endl;

      for (int k=0; k<objects.size(); k++) {
	string object = objects[k];
	//cout << "Comparing order for object [" << k << "]-" << object << endl;
	vector <int> vectorBordaUser,vectorUser;
	string lineA,lineB;
	getline(inputBordaUser, lineA); //This do not check if the file is incomplete or unstructured
	getline(inputFile, lineB); //This do not check if the file is incomplete or unstructured
	//cout << "Stringts:  A=" << lineA << "          B=" << lineB << endl;
	vectorBordaUser=extractOrder(lineA,roomsToEvaluate);
	vectorUser=extractOrder(lineB,roomsToEvaluate);
	//cout << "Vectors: " << printVector(A) << "      " << printVector(B) << endl;
	//double kendallTau=kendallTauNormalized( vectorBordaUser, vectorUser );
	//sumKendallTau+=kendallTau;
	if (vectorBordaUser[i]==vectorUser[i]) sumRoomMatch++;
	//cout << "Kendall Tau normalized distance from A order to B order: " <<  kendallTau << endl;
      }
      inputBordaUser.close();
      inputFile.close();
      cout << "Comparing " << titles[j] << endl;
      //cout << "Average Kendall Tau: " << sumKendallTau/objects.size() << " for " << objects.size() << " objects" << endl;
      cout << "Percentage of room match: " << sumRoomMatch/( (double)objects.size() ) << endl;
      //outputKendallTau << sumKendallTau/objects.size();
      outputUsersMatchingRooms << sumRoomMatch/( (double)objects.size() );
      if (j<elementsToCompare-1) {
	outputUsersMatchingRooms << ",";
      }
    }
    outputUsersMatchingRooms << endl;
  }
  outputUsersMatchingRooms.close();
}



void compareKendallTau(vector<string> rooms, vector<string> objects) {
  int elementsToCompare=17; //The number of methods (files) to be analized
  int roomsToEvaluate=10; //Consider only rooms with ID less than this
  int metricToCompareByObject=9; //Compare by object all orders versus BordaUser (Only compare one of all)
  const char *titlesVector[] = {"Google", "DBPedia", "ConceptNet5", "Word2Vec", "Random", "Fixed", "BordaWeb", "ProbabilityWeb", "WeightedProbabilityWeb", "BordaUser", "User1", "User2", "User3", "User4", "User5", "User6", "User7"};
  vector<string> titles(titlesVector, titlesVector+elementsToCompare);
  const char *filesVector[] = {"data/experimentQueryGoogle.txt", "data/experimentQueryDBPedia.txt", "data/experimentQueryConceptNet5.txt", "data/experimentQueryWord2Vec.txt", "data/experimentQueryRandom.txt", "data/experimentQueryFixed.txt", "data/experimentQueryBordaWeb.txt", "data/experimentQueryProbabilityWeb.txt", "data/experimentQueryWeightedProbabilityWeb.txt", "data/bordaUser.txt", "data/users/ramon.txt", "data/users/oac270676.txt", "data/users/palomizee.txt", "data/users/La Torres .txt", "data/users/2077.txt", "data/users/A5FDC06321.txt", "data/users/Ricardo Mastachi.txt"};
  vector<string> files(filesVector, filesVector+elementsToCompare);
  
  //Generate combinatories orders for web querys
  vector<string> votersFilesWeb;
  for (int i=0; i<4 ; i++) votersFilesWeb.push_back(files[i]);
  orderFromBorda(rooms, objects, votersFilesWeb, files[6]);
  orderFromProbabilitiy(rooms, objects, votersFilesWeb, files[7]);
  
  //static const double weightsTMP[] = {0.23,0.24,0.27,0.24}; //Normal the first one
  static const double weightsTMP[] = {0.2083333708, 0.272435698, 0.2596155233, 0.2596154079}; //From global results
  
  //static const double weightsTMP[] = {0.19,0.22,0.33,0.24}; //Diferencia aumentada 10 veces
  vector<double> weights (weightsTMP, weightsTMP + sizeof(weightsTMP) / sizeof(weightsTMP[0]) );
  orderFromWeightedProbabilitiy(rooms, objects, votersFilesWeb, files[8], weights);
  
  //Generate combinatories orders for users
  vector<string> votersFilesUsers;
  for (int i=10; i<17 ; i++) votersFilesUsers.push_back(files[i]);
  orderFromBorda(rooms, objects, votersFilesUsers, files[9]);
  //orderFromProbability is not possible in users due to users didn't capture probabilities
  
  //start comparing global results
  ofstream outputKendallTau("data/experimentsKendallTau.txt");
  ofstream outputWeightedKendallTau("data/experimentsWeightedKendallTau.txt");
  for (int i=0; i<elementsToCompare; i++) { //Write field titles in the results files
    outputKendallTau << titles[i];
    outputWeightedKendallTau << titles[i];
    if (i<elementsToCompare-1) {
      outputKendallTau << ",";
      outputWeightedKendallTau << ",";
    }
  }
  outputKendallTau << endl;
  outputWeightedKendallTau << endl;
  for (int i=0; i<elementsToCompare; i++) {
    for (int j=0; j<elementsToCompare; j++) {
      double sumKendallTau=0;
      double sumWeightedKendallTau=0;
      ifstream inputFileA(files[i].c_str());
      ifstream inputFileB(files[j].c_str());
      for (int k=0; k<objects.size(); k++) {
	string object = objects[k];
	//cout << "Comparing order for object [" << k << "]-" << object << endl;
	vector <int> A,B;
	string lineA,lineB;
	getline(inputFileA, lineA); //This do not check if the file is incomplete or unstructured
	getline(inputFileB, lineB); //This do not check if the file is incomplete or unstructured
	//cout << "Strings: " << lineA << "          " << lineB << endl;
	A=extractOrder(lineA,roomsToEvaluate);
	B=extractOrder(lineB,roomsToEvaluate);
	//cout << "Vectors: " << printVector(A) << "      " << printVector(B) << endl;
	double kendallTau=kendallTauNormalized( A, B );
	double weightedKendallTau=weightedKendallTauNormalized( A, B );
	sumKendallTau+=kendallTau;
	sumWeightedKendallTau+=weightedKendallTau;
	//cout << "Kendall Tau normalized distance from A order to B order: " <<  kendallTau << endl;
      }
      inputFileA.close();
      inputFileB.close();
      cout << "Comparing " << titles[i] << " with " << titles[j] << endl;
      cout << "Average Kendall Tau: " << sumKendallTau/objects.size() << " for " << objects.size() << " objects" << endl;
      cout << "Average Weighted Kendall Tau: " << sumWeightedKendallTau/objects.size() << " for " << objects.size() << " objects" << endl;
      
      outputKendallTau << sumKendallTau/objects.size();
      outputWeightedKendallTau << sumWeightedKendallTau/objects.size();
      if (j<elementsToCompare-1) {
	outputKendallTau << ",";
	outputWeightedKendallTau << ",";
      }
    }
    outputKendallTau << endl;
    outputWeightedKendallTau << endl;
  }
  outputKendallTau.close();
  outputWeightedKendallTau.close();
  
  
  //start comparing by object results
  ifstream inputFileA(files[metricToCompareByObject].c_str());
  ifstream inputFileB[ elementsToCompare ];
  for (int i=0; i<elementsToCompare; i++) {
    inputFileB[i].open( files[i].c_str() );
  }
  ofstream outputObjectKendallTau("data/experimentsObjectKendallTau.txt");
  ofstream outputObjectWeightedKendallTau("data/experimentsObjectWeightedKendallTau.txt");
  outputObjectKendallTau << "object,";
  outputObjectWeightedKendallTau << "object,";
  for (int i=0; i<elementsToCompare; i++) { //Write field titles in the results files
    outputObjectKendallTau << titles[i];
    outputObjectWeightedKendallTau << titles[i];
    if (i<elementsToCompare-1) {
      outputObjectKendallTau << ",";
      outputObjectWeightedKendallTau << ",";
    }
  }
  outputObjectKendallTau << endl;
  outputObjectWeightedKendallTau << endl;
  for (int i=0; i<objects.size(); i++) {
    outputObjectKendallTau << objects[i] << ",";
    outputObjectWeightedKendallTau << objects[i] << ",";
    vector <int> A;
    string lineA;
    getline(inputFileA, lineA); //This do not check if the file is incomplete or unstructured
    for (int j=0; j<elementsToCompare; j++) {
      vector <int> B;
      string lineB;
      getline(inputFileB[j], lineB); //This do not check if the file is incomplete or unstructured
      A=extractOrder(lineA, roomsToEvaluate);
      B=extractOrder(lineB, roomsToEvaluate);
      double kendallTau=kendallTauNormalized( A, B );
      double weightedKendallTau=weightedKendallTauNormalized( A, B );
      outputObjectKendallTau << kendallTau;
      outputObjectWeightedKendallTau << weightedKendallTau;
      if (j<elementsToCompare-1) {
	outputObjectKendallTau << ",";
	outputObjectWeightedKendallTau << ",";
      }
    }
    outputObjectKendallTau << endl;
    outputObjectWeightedKendallTau << endl;
  }
  inputFileA.close();
  for (int i=0; i<elementsToCompare; i++) {
    inputFileB[i].close();
  }
  outputObjectKendallTau.close();
  outputObjectWeightedKendallTau.close();
  
}


int main(int argc, char **argv) {
  
  /*
  //Basic rooms in a house
  const char *roomvector[] = {"kitchen","bedroom","bathroom","living room","dining room"};
  vector<string> rooms(roomvector, roomvector+5);
  */
  
  /*
  //rooms in article veloso, samadi
  const char *roomvector[] = {"bathroom","printer room","kitchen","office"};
  vector<string> rooms(roomvector, roomvector+4);
  */
  
  //Extended rooms
  const char *roomvector[] = {"kitchen","bedroom","bathroom","livingroom","diningroom","studio","playroom","patio","laundry","garage"};//hall
  vector<string> rooms(roomvector, roomvector+10);
  
  
  const char *objectvector[] = {"apple","shoe","coffee","clothe","laptop","bread","pen","book","bed sheet","cellphone","spoon","fork","glass of water","handbag","food","towel","chair","medicine","tv remote","coke","broom","mop","key","scissor","comb","bible","ac remote","cell charger","tablet","plate","backpack","baby bottle","headphone","nail clipper","jacket","hand cream","inhaler","cosmetic bag","fly swatter","pillow","blanket","milk","shirt","sock","cup","glasses","knife","soap","coat","pumpkin","orange","paddle","ball","dinosaur","bottle","toy car","frying pan","cd","dvd","videogame","toy","potato chips","cracker","cookie","extinguisher","phone","printer","potty","bookshelf","trash","fridge","softener"};
  vector<string> objects(objectvector, objectvector+72);
  
  
  //Compare results - Comment to make querys
  compareKendallTau(rooms, objects); //Compare kendall tau either weighted or not
  compareRoomMatching(rooms, objects);//Compare room coincidence by place in the order
  return 0;
  //compare results end
  
  /*
  //Calculate Kendall Tau Distance for an example
  static const int av[]={0,1,2,3,4,5,6,7,8,9};
  //static const int bv[]={0,1,2,3,4,5,6,7,8,9};
  //static const int bv[]={9,8,7,6,5,4,3,2,1,0};
  //static const int bv[]={8,9,7,6,5,4,3,2,1,0};
  //static const int bv[]={9,1,2,3,4,5,6,7,8,0};
  //static const int bv[]={1,0,2,3,4,5,6,7,8,9};
  static const int bv[]={0,1,2,3,4,5,6,7,9,8};
  vector <int> a (av, av + sizeof(av) / sizeof(av[0]) );
  vector <int> b (bv, bv + sizeof(bv) / sizeof(bv[0]) );
  cout << "Kendall Tau: " <<  kendallTauNormalized(a,b) << endl;
  cout << "Weighted Kendall Tau: " <<  weightedKendallTauNormalized(a,b) << endl;
  return 0;
  */
  
  
  
  //Making querys to internet experiments
  
  //For saving orders
  ofstream outputGoogle("data/experimentQueryGoogle.txt");
  ofstream outputDBPedia("data/experimentQueryDBPedia.txt");
  ofstream outputConcepNet5("data/experimentQueryConceptNet5.txt");
  ofstream outputWord2Vec("data/experimentQueryWord2Vec.txt");
  ofstream outputRandom("data/experimentQueryRandom.txt");
  ofstream outputFixed("data/experimentQueryFixed.txt");
  
  //For saving probabilities
  ofstream outputGooglePb("data/experimentQueryGooglePb.txt");
  ofstream outputDBPediaPb("data/experimentQueryDBPediaPb.txt");
  ofstream outputConcepNet5Pb("data/experimentQueryConceptNet5Pb.txt");
  ofstream outputWord2VecPb("data/experimentQueryWord2VecPb.txt");
  ofstream outputRandomPb("data/experimentQueryRandomPb.txt");
  ofstream outputFixedPb("data/experimentQueryFixedPb.txt");
  
  cout << "Starting at system time " << time(NULL) << " seconds" << endl;
  
  for (int i=0; i<objects.size(); i++) {
    
    vector <int> orderUser, orderGoogle, orderDBPedia, orderConceptNet5, orderWord2Vec, orderRandom, orderFixed;
    vector <double> probabilityUser, probabilityGoogle, probabilityDBPedia, probabilityConceptNet5, probabilityWord2Vec, probabilityRandom, probabilityFixed;
    
    string object = objects[i];
    cout << "Searching for object [" << i << "]-" << object << endl;
    
    //Capture an user distribution
    //orderFromUser(rooms, orderUser, probabilityUser, object);
    //cout << "Order from User: " << printVector(orderUser) << endl;
    //cout << "Probability from User: " << printVector(probabilityUser) << endl;
    
    
    orderFromGoogle(rooms, orderGoogle, probabilityGoogle, object);
    cout << "Order from Google: " << printVector(orderGoogle) << endl;
    cout << "Probability from Google: " << printVector(probabilityGoogle) << endl;
    cout << "Kendall Tau normalized distance from Google order to User order: " <<  kendallTauNormalized(orderUser,orderGoogle) << endl;
    outputGoogle << i << "-" << printVector(orderGoogle) << endl;
    outputGooglePb << object << "," << printVector(probabilityGoogle) << endl;
    
    orderFromDBPedia(rooms, orderDBPedia, probabilityDBPedia, object);
    cout << "Order from DBPedia: " << printVector(orderDBPedia) << endl;
    cout << "Probability from DBPedia: " << printVector(probabilityDBPedia) << endl;
    cout << "Kendall Tau normalized distance from DBPedia order to User order: " <<  kendallTauNormalized(orderUser,orderDBPedia) << endl;
    outputDBPedia << i << "-" << printVector(orderDBPedia) << endl;
    outputDBPediaPb << object << "," << printVector(probabilityDBPedia) << endl;
    
    orderFromConceptNet5(rooms, orderConceptNet5, probabilityConceptNet5, object);
    cout << "Order from ConceptNet5: " << printVector(orderConceptNet5) << endl;
    cout << "Probability from ConceptNet5: " << printVector(probabilityConceptNet5) << endl;
    cout << "Kendall Tau normalized distance from ConceptNet5 order to User order: " <<  kendallTauNormalized(orderUser,orderConceptNet5) << endl;
    outputConcepNet5 << i << "-" << printVector(orderConceptNet5) << endl;
    outputConcepNet5Pb << object << "," << printVector(probabilityConceptNet5) << endl;
    
    orderFromWord2Vec(rooms, orderWord2Vec, probabilityWord2Vec, object);
    cout << "Order from Word2Vec: " << printVector(orderWord2Vec) << endl;
    cout << "Probability from Word2Vec: " << printVector(probabilityWord2Vec) << endl;
    cout << "Kendall Tau normalized distance from Word2Vec order to User order: " <<  kendallTauNormalized(orderUser,orderWord2Vec) << endl;
    outputWord2Vec << i << "-" << printVector(orderWord2Vec) << endl;
    outputWord2VecPb << object << "," << printVector(probabilityWord2Vec) << endl;
    
    orderFromRandom(rooms, orderRandom, probabilityRandom, object);
    cout << "Order from Random: " << printVector(orderRandom) << endl;
    cout << "Probability from Random: " << printVector(probabilityRandom) << endl;
    cout << "Kendall Tau normalized distance from Random order to User order: " <<  kendallTauNormalized(orderUser,orderRandom) << endl;
    outputRandom << i << "-" << printVector(orderRandom) << endl;
    outputRandomPb << object << "," << printVector(probabilityRandom) << endl;
    
    orderFromFixed(rooms, orderFixed, probabilityFixed, object);
    cout << "Order from Fixed: " << printVector(orderFixed) << endl;
    cout << "Probability from Fixed: " << printVector(probabilityFixed) << endl;
    cout << "Kendall Tau normalized distance from Fixed order to User order: " <<  kendallTauNormalized(orderUser,orderFixed) << endl;
    outputFixed << i << "-" << printVector(orderFixed) << endl;
    outputFixedPb << object << "," << printVector(probabilityFixed) << endl;
    
    //wait some time for not to block the internet connection
    usleep(9000000);
    //End Making querys to internet experiments
    
  }
  
  //Free memory used by dictionary
  free(M);
  free(vocab);
  cout << "Ending at system time " << time(NULL) << " seconds" << endl;
  
  outputGoogle.close();
  outputDBPedia.close();
  outputConcepNet5.close();
  outputWord2Vec.close();
  outputRandom.close();
  outputFixed.close();
  
  outputGooglePb.close();
  outputDBPediaPb.close();
  outputConcepNet5Pb.close();
  outputWord2VecPb.close();
  outputRandomPb.close();
  outputFixedPb.close();
  return 0;
}

