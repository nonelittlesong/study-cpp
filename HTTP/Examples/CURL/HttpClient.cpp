#include <curl/curl.h>
#include <string>
#include <cstring>
#include <iostream>

int main(int argc, char *argv[]) {
  CURL *pCurl = NULL;
  CURLcode res = CURLE_OK;
  char resultJsonData[1024];
  memset(resultJsonData, 0, sizeof(resultJsonData));

  std::string strJson = "{";
  strJson += "\"id\":\"test\",";
  strJson += "\"resultStr\":1,";
  strJson += "\"totalOK\":1,";
  strJson += "\"totalNG\":0";
  strJson += "}";
  strcpy(resultJsonData, strJson.c_str());

  curl_slist *pHeaders = NULL;
  curl_slist_append(pHeaders, "Content-Type: application/json");
  curl_slist_append(pHeaders, "charset: utf-8");

  pCurl = curl_easy_init();
  if (pCurl)
  {
    curl_easy_setopt(pCurl, CURLOPT_URL, "http://127.0.0.1:100/api/result");
    curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 1);
    curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaders);
    
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, resultJsonData);
    /* if we don't provide POSTFIELDSIZE, libcurl will strlen() by itself */
    curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, strlen(resultJsonData));

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(pCurl);
    
    /* Check for errors */
    if (res != CURLE_OK)
    {
      std::cerr << "curl_easy_perform() failed: "
                << curl_easy_strerror(res) << std::endl;
    }
    
    /* always cleanup */
    curl_easy_cleanup(pCurl);
  }
  return 0;
}
