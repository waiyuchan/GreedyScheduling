#include "utils.hpp"

void split(const string &src, const string &separator, vector<string> &dest)
{
    string str = src;
    string substring;
    string::size_type start = 0, index;
    dest.clear();
    index = str.find_first_of(separator, start);
    do
    {
        if (index != string::npos)
        {
            substring = str.substr(start, index - start);
            dest.push_back(substring);
            start = index + separator.size();
            index = str.find(separator, start);
            if (start == string::npos)
                break;
        }
    } while (index != string::npos);

    substring = str.substr(start);
    dest.push_back(substring);
}

bool getDemand(const string path, vector<unordered_map<string, int> > &demands)
{
    bool is_empty = false;
    string user_ids, line, word;
    fstream file(path, ios::in);
    if (file.is_open())
    {
        is_empty = true;
        cout << "Demand file opened successfully." << endl;
        getline(file, user_ids);
        vector<string> ids;
        unsigned long index = 0;
        string id;

        while (index < user_ids.length())
        {
            if ((user_ids[index] != ',') && (user_ids[index] != '\r'))
            {
                id.push_back(user_ids[index]);
            }
            else
            {
                ids.push_back(id);
                id = "";
            }
            ++index;
        }
        ids.push_back(id);
        while (getline(file, line))
        {
            unordered_map<string, int> ump;
            string timestamp = line.substr(0, line.find(','));
            index = timestamp.length() + 1;
            string value = "";
            int i = 1;
            while (index < line.length())
            {
                if (line[index] != ',')
                {
                    value.push_back(line[index]);
                }
                else
                {
                    // cout << "客户节点: " << ids[i] << ": " << value << endl;
                    ump[ids[i]] = atoi(value.c_str());
                    value = "";
                    ++i;
                }
                ++index;
            }
            ump[ids[i]] = atoi(value.c_str());
            demands.push_back(ump);
        }
    }
    else
        cout << "Please check Demands file." << endl;

    return is_empty;
}

bool getQos(const string path, unordered_map<string, vector<int> > &qos)
{
    bool is_empty = false;
    string line, word;
    fstream file(path, ios::in);
    if (file.is_open())
    {
        is_empty = true;
        cout << "Qos file opened successfully." << endl;
        getline(file, line);
        while (getline(file, line))
        {
            vector<int> values;
            string key = line.substr(0, line.find(','));
            unsigned long index = key.length() + 1;
            string value;
            while (index < line.length())
            {
                if (line[index] != ',')
                {
                    value.push_back(line[index]);
                }
                else
                {
                    values.push_back(atoi(value.c_str()));
                    value = "";
                }
                ++index;
            }
            values.push_back(atoi(value.c_str()));
            qos[key] = values;
        }
    }
    else
        cout << "Please check Qos file" << endl;
    return is_empty;
}

bool getQosWithEdgeNode(const string path, unordered_map<string, unordered_map<string, int> > &qos_with_edge_node)
{
    bool is_empty = false;
    string line;
    fstream file(path, ios::in);

    if (file.is_open())
    {
        is_empty = true;
        cout << "Qos file with edge node opened successfully." << endl;
        getline(file, line);
        vector<string> client_nodes;
        unsigned long index = 0;
        string client_node;

        while (index < line.length())
        {
            if ((line[index] != ',') && (line[index] != '\r'))
            {
                client_node.push_back(line[index]);
            }
            else
            {
                client_nodes.push_back(client_node);
                client_node = "";
            }
            ++index;
        }
        client_nodes.push_back(client_node);

        vector<string>::iterator title = client_nodes.begin();
        client_nodes.erase(title);

        unordered_map<string, vector<int> > edge_node_qoses;

        while (getline(file, line))
        {
            vector<int> values;
            string key = line.substr(0, line.find(','));
            unsigned long index = key.length() + 1;
            string value;
            while (index < line.length())
            {
                if (line[index] != ',')
                {
                    value.push_back(line[index]);
                }
                else
                {
                    values.push_back(atoi(value.c_str()));
                    value = "";
                }
                ++index;
            }
            values.push_back(atoi(value.c_str()));
            edge_node_qoses[key] = values;
        }
        for (int i = 0; i < client_nodes.size(); i++)
        {
            unordered_map<string, int> qos_with_edge_and_client_node;
            for (unordered_map<string, vector<int> >::iterator iter = edge_node_qoses.begin(); iter != edge_node_qoses.end(); ++iter)
            {
                cout << client_nodes[i] << "-->  " << iter->first << ": " << iter->second[i] << endl;
                qos_with_edge_and_client_node[iter->first] = iter->second[i];
            }
            qos_with_edge_node[client_nodes[i]] = qos_with_edge_and_client_node;
        }
    }
    else
        cout << "Please check Qos file" << endl;
    return is_empty;
}

bool getQosConfig(const string path, int &qos_threshold)
{
    bool is_empty = false;
    string line;
    fstream file(path, ios::in);
    if (file.is_open())
    {
        cout << "Qos Config file opened successfully." << endl;
        getline(file, line); // skip the first line
        while (getline(file, line))
            qos_threshold = atoi(string(line.begin() + line.find('=') + 1, line.end()).c_str());
        is_empty = true;
    }
    else
    {
        cout << "Could not open the file" << endl;
    }
    return is_empty;
};

bool getBandwidth(const string path, unordered_map<string, int> &site_bandwidth)
{
    string line, word;
    bool flag = false;
    fstream file(path, ios::in);
    if (file.is_open())
    {
        cout << "BandWidth file opened successfully." << endl;
        getline(file, line); // skip the first line
        while (getline(file, line))
        {
            string key = line.substr(0, line.find(','));
            string value = line.substr(line.find(',') + 1);
            int value_int = stoi(value);
            site_bandwidth[key] = value_int;
        }
        flag = true;
    }
    else
        cout << "Could not open the file" << endl;
    return flag;
}

void writeAnswer(const string path, string &answer)
{
    ofstream write;
    write.open(path, ios::app);
    write << answer;
    write.close();
}
