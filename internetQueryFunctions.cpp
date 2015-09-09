#include <iostream>
#include<stdlib.h>
#include <curl/curl.h>  //Instalar paquete "libcurl4-gnutls-dev" desde Centro de software

using namespace std;


void removeCommas(string &strCommas) {
  size_t commaPos = strCommas.find(',',0);
  while (commaPos!=string::npos) {
    strCommas.erase(commaPos,1);
    commaPos = strCommas.find(',',0);
  }
}

void extractNumber(string &strNumber) {
  size_t numberPos = 0;
  while (numberPos<strNumber.length()) {
    if ( (strNumber[numberPos]>='0' && strNumber[numberPos]<='9') || strNumber[numberPos]=='.' )
      numberPos++;
    else
      strNumber.erase(numberPos,1);
  }
}

vector <string> extractWords(string strSearch){
  //words must be in lowercase
  locale loc;
  for (string::size_type i=0; i<strSearch.length(); ++i)
    strSearch[i]=tolower(strSearch[i],loc);
  
  vector <string> a;
  size_t spacePos = strSearch.find(' ',0);
  while (spacePos!=string::npos) {
    a.push_back( strSearch.substr(0,spacePos) );
    strSearch=strSearch.substr(spacePos+1);
    spacePos = strSearch.find(' ',0);
  }
  a.push_back( strSearch );
  return a;
}


void formatSearch(string &strSearch) {
  size_t spacePos = strSearch.find(' ',0);
  while (spacePos!=string::npos) {
    strSearch.erase(spacePos,1);
    strSearch.insert(spacePos,"+AND+",5);
    spacePos = strSearch.find(' ',0);
  }
  locale loc;
  for (string::size_type i=0; i<strSearch.length(); ++i)
    strSearch[i]=toupper(strSearch[i],loc);
}

string data; //will hold the url's contents

size_t writeCallback(char* buffer, size_t size, size_t nmemb, void* userp) {
  //callback must have this declaration
  //buffer is a pointer to the data that curl has for us
  //size*nmemb is the size of the buffer
  
  for (int c = 0; c<size*nmemb; c++) {
    data.push_back(buffer[c]);
  }
  return size*nmemb; //tell curl how many bytes we handled
}

/*
 * USAGE: googleHitsCount("apple+kitchen")
 */
long int googleHitsCount(string search) {
  CURL *curl;
  CURLcode res;
  
  string url="http://www.google.com/search?q=";
  string completeURL=url+search;
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, completeURL.c_str() );
    /* example.com is redirected, so we tell libcurl to follow redirection */ 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L); //tell curl to output its progress
 
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));
 
    /* always cleanup */ 
    curl_easy_cleanup(curl);
  }
  
  //resultStats is the word before the number of hits. There are two coincidences but only one with id="resultstats
  size_t startSTR = data.find("id=\"resultStats");
  size_t endSTR = data.find("</", startSTR+17);
  string hits;
  hits = data.substr( startSTR+17 , endSTR-(startSTR+17) );
  extractNumber(hits);
  long int nh=atol( hits.c_str() );
  
  data.clear();
  return nh;
}

/*
 * USAGE: dbpediaHitsCount("apple kitchen")
 */
long int dbpediaHitsCount(string search) {
  CURL *curl;
  CURLcode res;
  
  string url="http://dbpedia.org/sparql?";
  
  string postData = "default-graph-uri=http%3A%2F%2Fdbpedia.org&query";
  postData=postData+"=select+count%28*%29+where+{+%0D%0A++quad+map+";
  postData=postData+"virtrdf%3ADefaultQuadMap+{+%0D%0A++++graph+%3Fg+";
  postData=postData+"{+%0D%0A++++++%3Fs1+%3Fs1textp+%3Fo1+.%0D%0A";
  postData=postData+"++++++%3Fo1+bif%3Acontains+%27+%28+";
  
  formatSearch(search); //Change spaces between words for "+AND+" operator
  postData=postData+search;
  
  postData=postData+"+%29+%27+option+%28+score+%3Fsc+%29+.++++++++++++%0D";
  postData=postData+"%0A++++}%0D%0A++}%0D%0A}&format=text%2Fhtml&";
  postData=postData+"timeout=30000&debug=on";
  
  string completeURL=url+postData;
  
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, completeURL.c_str() );

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback );	
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L); //tell curl to output its progress with 1L
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
	      curl_easy_strerror(res));
      
      /* always cleanup */ 
      curl_easy_cleanup(curl);
  }
  
  ////The result is between the first "td" labels
  size_t startSTR = data.find("<td>")+4;
  size_t endSTR = data.find("</td>");
  string hits;
  hits = data.substr( startSTR , endSTR );
  removeCommas(hits);
  long int nh=atol( hits.c_str() );
  
  data.clear();
  return nh;
}


/*
 * USAGE: conceptNet5("apple kitchen")
 */
long int conceptNet5HitsCount(string search) {
  CURL *curl;
  CURLcode res;
  
  string url="http://conceptnet5.media.mit.edu/";
  
  string postData = "data/5.2/assoc/c/en/";
  vector <string> wordList;
  wordList=extractWords(search);
  postData=postData+wordList[0];
  for (int i=1;i<wordList.size();i++) {
    postData=postData+"?filter=/c/en/";
    postData=postData+wordList[i];
  }
  postData=postData+"&limit=1";
  
  string completeURL=url+postData;
  
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, completeURL.c_str() );
    
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback );	
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L); //tell curl to output its progress with 1L
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
	      curl_easy_strerror(res));
      
      /* always cleanup */ 
      curl_easy_cleanup(curl);
  }
  
  //The result is between "similar" and "term"
  size_t startSTR = data.find("similar")+6;
  size_t endSTR = data.find("term");
  string hits;
  hits = data.substr( startSTR , endSTR );
  extractNumber(hits);
  long int nh=(long int) (1000000 * atof( hits.c_str() ) );
  
  data.clear();
  return nh;
}

