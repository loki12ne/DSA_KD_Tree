#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
using namespace std;

const double R = 6371e3;
const double pi = 2 * acos(0);

struct City {
	string name;
    float lat, lng;
	City() {
		name = "";
		lat = lng = 0;
    }
};

struct KD_Tree {
    City city;
    KD_Tree* pLeft, * pRight;
    KD_Tree() {
        pLeft = pRight = NULL;
    }
};

City parseCity(string input) {
    stringstream ss(input);
    string token;
    City city;

    getline(ss, city.name, ',');

    getline(ss, token, ',');
    city.lat = stof(token);

    getline(ss, token, ',');
    city.lng = stof(token);

    getline(ss, token, ',');
    if (token[0] == '"')
        getline(ss, token, ',');

    getline(ss, token, ',');

    return city;
}

vector<City> readFile(string FileName) {
    ifstream fin(FileName);
    string s;

    getline(fin, s);

    vector<City> vec;
    while (getline(fin, s)) {
        City newCity = parseCity(s);
        vec.push_back(newCity);
    }
    return vec;
}



/////////////////////////////////////////////////////////////
// KD_TREE 
/////////////////////////////////////////////////////////////

KD_Tree* createNode(City city)
{
    KD_Tree* res = new KD_Tree;
    res->city = city;
    return res;
}

KD_Tree* insertNode(KD_Tree* root, City city, int depth) {
    if (!root) {
        return createNode(city);
    }

    // follow lat when depth is even
    if (!(depth % 2)) {
        if (root->city.lat < city.lat) {
            root->pRight = insertNode(root->pRight, city, depth + 1);
        }
        else {
            root->pLeft = insertNode(root->pLeft, city, depth + 1);
        }
    }
    // follow lng when depth is odd
    else {
        if (root->city.lng < city.lng) {
            root->pRight = insertNode(root->pRight, city, depth + 1);
        }
        else {
            root->pLeft = insertNode(root->pLeft, city, depth + 1);
        }
    }
    return root;
}

KD_Tree* createTree(vector<City> city){
    KD_Tree* root = NULL;
    int size = city.size();

    for (int i = 0; i < size; i++) {
    root = insertNode(root, city[i], 0);
    }

    return root;
}

///////////////////////////////////////////

double radian(float value) {
    return value * pi / 180;
}

float distance(City city1, City city2) {
    double lat1 = radian(city1.lat);
    double lng1 = radian(city1.lng);
    double lat2 = radian(city2.lat);
    double lng2 = radian(city2.lng);
    double a = pow(sin((lat2 - lat1) / 2), 2) + cos(lat1) * cos(lat2) * pow(sin((lng2 - lng1) / 2), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

void nearest_neighbor(KD_Tree* root, City city, double& min_des, City& result, int depth) {
    if (root == NULL) {
        return;
    }

    double dist = distance(root->city, city);
    if (dist < min_des && city.name != root->city.name) {
        min_des = dist;
        result = root->city;
    }
    KD_Tree* nextBranch = NULL;
    KD_Tree* oppositeBranch = NULL;
    if (!(depth % 2)) {
        if (city.lat < root->city.lat) {
            nextBranch = root->pLeft;
            oppositeBranch = root->pRight;
        }
        else {
            nextBranch = root->pRight;
            oppositeBranch = root->pLeft;
        }
    }
    else {
        if (city.lng < root->city.lng) {
            nextBranch = root->pLeft;
            oppositeBranch = root->pRight;
        }
        else {
            nextBranch = root->pRight;
            oppositeBranch = root->pLeft;
        }
    }
    nearest_neighbor(nextBranch, city, min_des, result, depth + 1);
    double temp;
    if (!(depth % 2)) {
        City c;
        c.name = "";
        c.lat = root->city.lat;
        c.lng = city.lng;
        temp = distance(c, city);
    }
    else {
        City c;
        c.name = "";
        c.lat = city.lat;
        c.lng = root->city.lng;
        temp = distance(c, city);
    }
    if (temp < min_des) {
        nearest_neighbor(oppositeBranch, city, min_des, result, depth + 1);
    }
}

///////////////////////////////////////////

bool inside(City city, City min, City max)
{
    return (city.lat >= min.lat && city.lng >= min.lng && city.lat <= max.lat && city.lng <= max.lng);
}

void rangeSearch(KD_Tree* root, City min, City max, int level, vector<City>& range)
{
    if (!root) return;

    if (inside(root->city, min, max)) range.push_back(root->city);
    if (level % 2 != 0)
    {
        if (root->city.lng < min.lng) rangeSearch(root->pRight, min, max, level + 1, range);
        else if (root->city.lng > max.lng) rangeSearch(root->pLeft, min, max, level + 1, range);
        else
        {
            rangeSearch(root->pRight, min, max, level + 1, range);
            rangeSearch(root->pLeft, min, max, level + 1, range);
        }
    }
    else
    {
        if (root->city.lat < min.lat) rangeSearch(root->pRight, min, max, level + 1, range);
        else if (root->city.lat > max.lat) rangeSearch(root->pLeft, min, max, level + 1, range);
        else
        {
            rangeSearch(root->pRight, min, max, level + 1, range);
            rangeSearch(root->pLeft, min, max, level + 1, range);
        }
    }


}

KD_Tree* deserialize(ifstream& fin)
{
    int name_length;
    int lat_length, lng_length;
    fin.read((char*)&name_length, sizeof(name_length));
    if (name_length == 0)
        return nullptr;
    char* tmp = new char[name_length];
    fin.read(tmp, name_length);
    tmp[name_length - 1] = '\0'; // 

    if (strcmp(tmp, "invalid") == 0)
    {
        delete[] tmp; // 
        return nullptr;
    }
    fin.read((char*)&lat_length, sizeof(lat_length));
    fin.read((char*)&lng_length, sizeof(lng_length));

    KD_Tree* root = new KD_Tree;
    root->city.name = tmp;
    delete[] tmp; // 

    fin.read((char*)&root->city.lat, sizeof(root->city.lat));
    fin.read((char*)&root->city.lng, sizeof(root->city.lng));
    root->pLeft = deserialize(fin);
    root->pRight = deserialize(fin);

    return root;
}

void serialize(KD_Tree*& root, ofstream& fout)
{
    if (root == NULL)
    {
        char tmp[] = "invalid";
        int size = strlen(tmp) + 1;
        fout.write((char*)&size, sizeof(size));
        fout.write(tmp, size);
        return;
    }

    string tmp = root->city.name;
    const char* s = tmp.c_str();
    int name_length = tmp.size() + 1;
    int lat_length = sizeof(root->city.lat);
    int lng_length = sizeof(root->city.lng);

    fout.write((char*)&name_length, sizeof(name_length));
    fout.write(s, name_length);
    fout.write((char*)&lat_length, sizeof(lat_length));
    fout.write((char*)&lng_length, sizeof(lng_length));
    fout.write((char*)&root->city.lat, lat_length);
    fout.write((char*)&root->city.lng, lng_length);

    serialize(root->pLeft, fout);
    serialize(root->pRight, fout);
}



///////////////////////////////////////////
// INTERFACE
///////////////////////////////////////////


KD_Tree* loadCity()
{
    string file_name;
    cout << "\nEnter filename: ";
    cin.ignore();
    getline(cin, file_name);

    vector<City> tmp = readFile(file_name);
    KD_Tree* res = NULL;
    res = createTree(tmp);
    return res;

}

void InsertNewCity(KD_Tree*& node)
{
    City newCity;
    string token;
    cin.ignore();
    cout << "\nEnter name: "; getline(cin, newCity.name);
    cout << "\nEnter latitude: "; cin >> newCity.lat;
    cout << "\nEnter longtitude: "; cin >> newCity.lng;
    node = insertNode(node, newCity, 0);
    cout << "\n\nEnter to continue: ";
    cin.ignore();
    getline(cin, token);
}

void InsertMulti(KD_Tree*& node)
{
    string file_path;
    cin.ignore();
    string token;
    cout << "\nEnter path: ";
    getline(cin, file_path);

    vector<City> tmp = readFile(file_path);

    int size = tmp.size();
    for (int i = 0; i < size; i++) {
        node = insertNode(node, tmp[i], 0);
    }
    cout << "\n\nEnter to continue: ";
    getline(cin, token);

}

void NearestSearch(KD_Tree* node)
{
    if (!node)
        return;
    City newCity;
    string token;
    cout << "\nEnter latitude: "; cin >> newCity.lat;
    cout << "\nEnter longtitude: "; cin >> newCity.lng;

    City res = node->city;
    double des = distance(newCity, node->city);
    nearest_neighbor(node, newCity, des , res, 0);
    cout << "\n" << res.name << ": " << des;
    cin.ignore();
    cout << "\n\nEnter to continue: ";
    getline(cin, token);
}


void print(KD_Tree* node)
{
    if (!node)
        return;
    print(node->pLeft);
    cout << node->city.name << " " << node->city.lat << " " << node->city.lng << "\n";
    print(node->pRight);
}

void printToFile(KD_Tree* node, ofstream& fp)
{
    if (!node)
        return;
    printToFile(node->pLeft, fp);
    fp << node->city.name << "," << node->city.lat << "," << node->city.lng << "\n";
    printToFile(node->pRight, fp);
}

void RangeSearch(KD_Tree* node)
{
    vector<City> res;
    City min, max;
    string token;

    cout << "\nEnter latitude: "; cin >> min.lat;
    cout << "\nEnter longtitude: "; cin >> min.lng;
    cout << "\nEnter latitude: "; cin >> max.lat;
    cout << "\nEnter longtitude: "; cin >> max.lng;
    
    if (min.lat > max.lat) swap(min.lat, max.lat);
    if (min.lng > max.lng) swap(min.lng, max.lng);

    rangeSearch(node, min, max, 0, res);
    int size = res.size();
    if (size == 0)
        cout << "NO COUNTRY";
    for (int i = 0;i < size; i++)
        cout << res[i].name << "\n";
    cout << "\n\nEnter to continue: ";
    cin.ignore();
    getline(cin, token);
}


void WriteCitytoFile(KD_Tree* root)
{
    string filename;
    cout << "\nEnter file name to save kd tree: ";
    cin >> filename;
    ofstream fout(filename, ios::binary);
    if (!fout)
    {
        cerr << "\nCan not open file";
        return;
    }
    serialize(root, fout);
    fout.close();
    cout << "\nfile saved successfully!\n";
    cout << "\n\nEnter to continue: ";
    string token;
    cin.ignore();
    getline(cin, token);
}

void Output(KD_Tree* node)
{
    int choice;
    cout << "\n1. To console";
    cout << "\n2. To file";
    cout << "\nEnter your choice: ";
    cin >> choice;
    if (choice == 1)
    {
        cout << "\nCityName - Latitude - Longitude\n";
        print(node);
    }
    else if (choice == 2)
    {
        string filename;
        cout << "\nEnter file name: ";
        cin.ignore();
        getline(cin, filename);
        ofstream fp(filename);
        fp << "city_name, latitude, longitude\n";
        printToFile(node, fp);

    }
    cout << "\n\nEnter to continue: ";
    string token;
    cin.ignore();
    getline(cin, token);
}

void ReconstructKD_Tree(KD_Tree*& root)
{
    string filename;
    cout << "\nEnter file name: ";
    cin >> filename;
    ifstream fin(filename, ios::binary);
    if (!fin)
    {
        cerr << "\nCannot open file";
        return;
    }
    root = deserialize(fin);
    cout << "\nReconstructed tree successfully!";
    fin.close();
    cout << "\n\nEnter to continue: ";
    string token;
    cin.ignore();
    getline(cin, token);
}
void userInterface()
{
    KD_Tree* node = NULL;
    while (true)
    {
        system("cls");
        cout << "===========================================\n";
        cout << "0. Output\n";
        cout << "1. Load cities from a file\n";
        cout << "2. Insert a new city\n";
        cout << "3. Insert multiple new cities\n";
        cout << "4. Nearest neighbor search\n";
        cout << "5. Range search\n";
        cout << "6. Save to file\n";
        cout << "7. Deserializing the KD-Tree from file\n";
        cout << "8. Exit\n";
        cout << "============================================\n";
        cout << "Enter your choice: ";
        int choice;
        cin >> choice;
        if (choice < 0 || choice > 8)
        {
            continue;
        }
        else if (choice == 8)
        {
            cout << "YOU CHOOSE EXIT";
            break;
        }
        else
        {
            switch (choice)
            {
            case 0:
                Output(node);
                break;
            case 1:
                node = loadCity();
                break;
            case 2:
                InsertNewCity(node);
                break;
            case 3:
                InsertMulti(node);
                break;
            case 4:
                NearestSearch(node);
                break;
            case 5:
                RangeSearch(node);
                break;
            case 6:
                WriteCitytoFile(node);
                break;
            case 7:
                ReconstructKD_Tree(node);
                break;
            }
        }
    }

}


int main()
{
    userInterface();
}
