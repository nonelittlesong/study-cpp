#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>

int main()
{
	std::ifstream inf("fold_0_data.txt");
	std::string user_id;
	std::string original_image;
	int face_id;
	std::string minAge, maxAge;
	char gender;
	char discardString[256];
    
    std::map<char, int> genderID;
	genderID.insert(std::pair<char, int>('m', 0));
	genderID.insert(std::pair<char, int>('f', 1));

	std::map<std::string, int> ageID;
	ageID.insert(std::pair<std::string, int>("(0,", 0));
	ageID.insert(std::pair<std::string, int>("(4,", 0));
	ageID.insert(std::pair<std::string, int>("(8,", 0));
	ageID.insert(std::pair<std::string, int>("(15,", 1));
	ageID.insert(std::pair<std::string, int>("(25,", 2));
	ageID.insert(std::pair<std::string, int>("(38,", 2));
	ageID.insert(std::pair<std::string, int>("(48,", 3));
	ageID.insert(std::pair<std::string, int>("(60,", 4));

	// new data
    std::vector<std::string> newData;
	std::ofstream outf("newdata.txt");
	
	inf.getline(discardString, 256);

	while(inf.peek()!=EOF)
	{
		inf >> user_id >> original_image >> face_id >> minAge >> maxAge >> gender;
		inf.getline(discardString, 256);
        
		// skip age = None, gender = u
		if (minAge.compare(0, 1, "(")!=0 || gender == 'u') continue;
		// analyse data
        std::cout << user_id << "\n"
			      << original_image << "\n"
				  << minAge << "\n"
				  << gender << std::endl;
        // write new data
		newData.push_back("faces/"+user_id+"/coarse_tilt_aligned_face."+std::to_string(face_id)+"."+original_image + " "
				          + std::to_string(ageID.find(minAge)->second) + " " 
						  + std::to_string(genderID.find(gender)->second) + "\n");
	}

    std::random_shuffle(newData.begin(), newData.end());
	for (std::vector<std::string>::iterator it = newData.begin(); it != newData.end(); ++it)
	{
		outf << *it;
	}
	outf.close();
	return 0;
}
