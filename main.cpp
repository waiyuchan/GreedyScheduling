#include "utils.cpp"

int main()
{
    string folder_path = "../data/";
    // string folder_path = "../pressure_data/";
    string demand_filename = folder_path + "demand.csv";
    string qos_filename = folder_path + "qos.csv";
    string qos_config_filename = folder_path + "config.ini";
    string site_bandwidth_filename = folder_path + "site_bandwidth.csv";
    string output_path = "../output/solution.txt";

    vector<unordered_map<string, int> > demands;
    unordered_map<string, vector<int> > qoses;
    unordered_map<string, unordered_map<string, int> > qos_with_edge_node;
    int qos_threshold;
    unordered_map<string, int> site_bandwidths;

    bool is_demand_empty = getDemand(demand_filename, demands);
    bool is_qos_empty = getQos(qos_filename, qoses);
    bool is_qos_with_edge_node_empty = getQosWithEdgeNode(qos_filename, qos_with_edge_node);
    bool is_qos_config_empty = getQosConfig(qos_config_filename, qos_threshold);
    bool is_site_bandwidths_empty = getBandwidth(site_bandwidth_filename, site_bandwidths);

    int top_five_percent_number = 0; // top 5%的免费额度数量

    unordered_map<string, unordered_map<string, int> > valid_qos_with_edge_node = qos_with_edge_node;
    unordered_map<string, vector<int> > edge_nodes_cost_list;
    vector<int> empty_cost;

    if (is_demand_empty && is_qos_empty && is_qos_with_edge_node_empty && is_qos_config_empty && is_site_bandwidths_empty)
    {
        cout << "Data file read successfully!" << endl;
        int s_times = demands.size(); // s_times: Scheduling times, 流量调度次数
        top_five_percent_number = floor(s_times * 0.05);
        cout << "免费额度数量：" << top_five_percent_number << endl;

        // 为每个边缘节点分配top 5%调度数量的最大限额
        unordered_map<string, int> edge_nodes_with_no_cost_times;
        unordered_map<string, int>::iterator b_iter = site_bandwidths.begin();
        for (; b_iter != site_bandwidths.end(); ++b_iter)
            edge_nodes_with_no_cost_times[b_iter->first] = top_five_percent_number;

        // 过滤掉不满足客户节点Qos限制要求的边缘节点
        unordered_map<string, unordered_map<string, int> >::iterator client_iter = qos_with_edge_node.begin();
        for (; client_iter != qos_with_edge_node.end(); ++client_iter)
        {
            unordered_map<string, int>::iterator edge_node_iter = client_iter->second.begin();
            for (; edge_node_iter != client_iter->second.end(); ++edge_node_iter)
            {
                if (edge_node_iter->second >= qos_threshold)
                    valid_qos_with_edge_node[client_iter->first].erase(edge_node_iter->first);
            }
        }

        // 映射有效边缘节点和客户节点的关系
        unordered_map<string, vector<string> > valid_edge_nodes_for_client;
        unordered_map<string, unordered_map<string, int> >::iterator tmp_client_iter = valid_qos_with_edge_node.begin();
        for (; tmp_client_iter != qos_with_edge_node.end(); ++tmp_client_iter)
        {
            unordered_map<string, int>::iterator tmp_edge_node_iter = tmp_client_iter->second.begin();
            vector<string> edge_nodes;
            for (; tmp_edge_node_iter != tmp_client_iter->second.end(); ++tmp_edge_node_iter)
                edge_nodes.push_back(tmp_edge_node_iter->first);
            valid_edge_nodes_for_client[tmp_client_iter->first] = edge_nodes;
        }

        string answers;

        // for (int stime = 0; stime < 2; stime++)
        for (int stime = 0; stime < s_times; stime++)
        {
            // 每次的带宽限额都是初始的
            unordered_map<string, int> tmp_site_bandwidths = site_bandwidths;
            unordered_map<string, unordered_map<string, int> > result = qos_with_edge_node;
            vector<string> full_loaded_edge_nodes;
            // 某时刻的客户节点流量需求总量计算
            cout << "第 " << stime << " 次流量调度: " << endl;

            // 使用贪心策略找到最少成本
            unordered_map<string, int>::iterator demand_iter = demands[stime].begin();
            unordered_map<string, unordered_map<string, int> >::iterator q_iter = qos_with_edge_node.begin();

            for (; q_iter != qos_with_edge_node.end(); ++q_iter)
            {
                unordered_map<string, int>::iterator r_iter = q_iter->second.begin();
                for (; r_iter != q_iter->second.end(); ++r_iter)
                    result[q_iter->first][r_iter->first] = 0;
            }

            for (; demand_iter != demands[stime].end(); ++demand_iter)
            {
                vector<string> valid_edge_nodes = valid_edge_nodes_for_client[demand_iter->first];
                int valid_edge_nodes_num = valid_edge_nodes.size();
                int require_flow = demand_iter->second;
                string client_node = demand_iter->first;
                if (client_node.find("\n") != string::npos || client_node.find("\r\n") != string::npos)
                {
                    client_node = client_node.substr(0, client_node.length() - 1);
                }

                cout << "客户节点 " << client_node << " 的流量需求为 : " << require_flow << endl;
                cout << "客户节点 " << client_node << " 的可用节点为 ";
                for (int i = 0; i < valid_edge_nodes_num; i++)
                    cout << valid_edge_nodes[i] << "(带宽: " << tmp_site_bandwidths[valid_edge_nodes[i]] << ", 免费次数: " << edge_nodes_with_no_cost_times[valid_edge_nodes[i]] << ((i == valid_edge_nodes.size() - 1) ? ")" : "), ");
                cout << endl;

                if (require_flow != 0)
                {
                    // 优先消耗top 5%的边缘节点用来规避成本
                    for (int i = 0; i < valid_edge_nodes_num; i++)
                    {
                        int bandwidths = tmp_site_bandwidths[valid_edge_nodes[i]];
                        int no_cost_times = edge_nodes_with_no_cost_times[valid_edge_nodes[i]];
                        if (require_flow != 0)
                        {
                            cout << "======>  剩余待分配流量: " << require_flow << " , 边缘节点 " << valid_edge_nodes[i] << "(带宽: " << tmp_site_bandwidths[valid_edge_nodes[i]] << ", 免费次数: " << edge_nodes_with_no_cost_times[valid_edge_nodes[i]] << ")" << endl;
                            if (no_cost_times > 0)
                            {
                                cout << "边缘节点 " << valid_edge_nodes[i] << " 还有 " << edge_nodes_with_no_cost_times[valid_edge_nodes[i]] << " 次免费次数，可以优先分配, ";
                                if (bandwidths > require_flow)
                                {
                                    cout << "剩余宽带上限足够, ";
                                    tmp_site_bandwidths[valid_edge_nodes[i]] -= require_flow;
                                    cout << "已分配流量: " << to_string(require_flow) << endl;
                                    result[client_node][valid_edge_nodes[i]] += require_flow;
                                    require_flow = 0;
                                    if (count(full_loaded_edge_nodes.begin(), full_loaded_edge_nodes.end(), valid_edge_nodes[i]) == 0)
                                        full_loaded_edge_nodes.push_back(valid_edge_nodes[i]);
                                }
                                else if (0 < bandwidths && bandwidths <= require_flow)
                                {
                                    if (bandwidths == require_flow)
                                    {
                                        cout << "剩余宽带: " << to_string(bandwidths) << " ，上限刚好足够, ";
                                        tmp_site_bandwidths[valid_edge_nodes[i]] -= require_flow;
                                        cout << "已分配流量: " << to_string(require_flow) << endl;
                                        result[client_node][valid_edge_nodes[i]] += require_flow;
                                        require_flow = 0;
                                        if (count(full_loaded_edge_nodes.begin(), full_loaded_edge_nodes.end(), valid_edge_nodes[i]) == 0)
                                            full_loaded_edge_nodes.push_back(valid_edge_nodes[i]);
                                    }
                                    else
                                    {
                                        cout << "剩余宽带: " << to_string(bandwidths) << " ，上限不足, ";
                                        require_flow -= bandwidths;
                                        cout << "已分配流量: " << to_string(bandwidths) << " ，剩余未分配流量: " << to_string(require_flow) << endl;
                                        result[client_node][valid_edge_nodes[i]] += bandwidths;
                                        tmp_site_bandwidths[valid_edge_nodes[i]] = 0;
                                        if (count(full_loaded_edge_nodes.begin(), full_loaded_edge_nodes.end(), valid_edge_nodes[i]) == 0)
                                            full_loaded_edge_nodes.push_back(valid_edge_nodes[i]);
                                    }
                                }
                                else
                                    cout << "但剩余宽带已消耗完毕，为 " << to_string(bandwidths) << " ，故不参与分配，寻找下一个可分配的节点" << endl;
                            }
                            else
                                cout << "边缘节点 " << valid_edge_nodes[i] << " 没有免费次数了，不优先分配，寻找下一个可免费的节点" << endl;
                        }
                        else
                            break;
                    }

                    if (require_flow != 0)
                    {
                        cout << "需求流量还未分配完，剩余: " << require_flow << endl;
                        int avg_flow = require_flow / valid_edge_nodes_num;
                        int extra_flow = require_flow % valid_edge_nodes_num;
                        cout << "所有边缘节点的均分流量为: " << avg_flow << " ，最后一个边缘节点需要承担更多一些，保证全部分配完" << endl;
                        for (int i = 0; i < valid_edge_nodes_num; i++)
                        {
                            int bandwidths = tmp_site_bandwidths[valid_edge_nodes[i]];
                            if (i != valid_edge_nodes_num - 1)
                            {
                                if (bandwidths > avg_flow)
                                {
                                    tmp_site_bandwidths[valid_edge_nodes[i]] -= avg_flow;
                                    require_flow -= avg_flow;
                                    result[client_node][valid_edge_nodes[i]] += avg_flow;
                                }
                                else
                                {
                                    require_flow -= tmp_site_bandwidths[valid_edge_nodes[i]];
                                    result[client_node][valid_edge_nodes[i]] += tmp_site_bandwidths[valid_edge_nodes[i]];
                                    extra_flow += avg_flow - tmp_site_bandwidths[valid_edge_nodes[i]];
                                    tmp_site_bandwidths[valid_edge_nodes[i]] = 0;
                                }
                            }
                            else
                            {
                                if (bandwidths > avg_flow + extra_flow)
                                {
                                    tmp_site_bandwidths[valid_edge_nodes[i]] -= avg_flow + extra_flow;
                                    require_flow -= avg_flow + extra_flow;
                                    result[client_node][valid_edge_nodes[i]] += avg_flow + extra_flow;
                                }
                                else
                                {
                                    require_flow -= tmp_site_bandwidths[valid_edge_nodes[i]];
                                    result[client_node][valid_edge_nodes[i]] += tmp_site_bandwidths[valid_edge_nodes[i]];
                                    extra_flow = avg_flow + extra_flow - tmp_site_bandwidths[valid_edge_nodes[i]];
                                    tmp_site_bandwidths[valid_edge_nodes[i]] = 0;

                                    int j = 0;
                                    while (extra_flow != 0)
                                    {
                                        int bandwidths = tmp_site_bandwidths[valid_edge_nodes[j]];
                                        if (bandwidths != 0)
                                        {
                                            if (bandwidths > extra_flow)
                                            {
                                                tmp_site_bandwidths[valid_edge_nodes[j]] -= extra_flow;
                                                result[client_node][valid_edge_nodes[j]] += extra_flow;
                                                extra_flow = 0;
                                                require_flow = 0;
                                            }
                                            else
                                            {
                                                result[client_node][valid_edge_nodes[j]] += tmp_site_bandwidths[valid_edge_nodes[j]];
                                                extra_flow -= tmp_site_bandwidths[valid_edge_nodes[j]];
                                                tmp_site_bandwidths[valid_edge_nodes[j]] = 0;
                                                j++;
                                            }
                                        }
                                        else
                                            j++;
                                    }
                                }
                            }
                        }
                    }

                    if (require_flow == 0)
                        cout << "* 该客户需要的所有流量已经分配完成!" << endl;
                    else
                        cout << "**** 该客户需要的所有流量还未分配完成!!!!!!" << endl;

                    cout << "-------------------" << endl;
                    cout << "分配方案：" << endl;
                    string answer = client_node + ":";
                    for (unordered_map<string, unordered_map<string, int> >::iterator result_iter = result.begin(); result_iter != result.end(); ++result_iter)
                    {
                        unordered_map<string, int>::iterator r_iter = result_iter->second.begin();
                        if (result_iter->first == client_node)
                        {
                            for (; r_iter != result_iter->second.end(); ++r_iter)
                            {
                                if (r_iter->second != 0)
                                {
                                    answer += "<" + r_iter->first + "," + to_string(r_iter->second) + ">,";
                                    edge_nodes_cost_list[r_iter->first].push_back(r_iter->second);
                                }
                                else
                                    edge_nodes_cost_list[r_iter->first].push_back(0);
                            }
                        }
                    }
                    cout << endl;
                    answer = answer.substr(0, answer.length() - 1) + "\n";
                    answers += answer;
                    cout << answer << endl;
                }
                else
                {
                    cout << "-------------------" << endl;
                    cout << "分配方案：" << endl;
                    cout << client_node << ":";
                    string answer = client_node + ":";
                    answers += answer + "\n";
                    cout << answer << endl;
                }
            }
            cout << "------------------------------------------------------------------" << endl;
            cout << "满载的节点有：" << endl;
            for (int i = 0; i < full_loaded_edge_nodes.size(); i++)
            {
                cout << full_loaded_edge_nodes[i] << ((i == full_loaded_edge_nodes.size() - 1) ? "" : ", ");
                edge_nodes_with_no_cost_times[full_loaded_edge_nodes[i]]--;
            }
            cout << endl;
        }
        answers = answers.substr(0, answers.size() - 1);
        writeAnswer(output_path, answers);

        // 成本计算
        int cost_location = ceil(demands.size() * 0.95);
        int cost = 0;
        unordered_map<string, vector<int> >::iterator encl_iter = edge_nodes_cost_list.begin();
        cout << "---------------------" << endl;
        for (; encl_iter != edge_nodes_cost_list.end(); ++encl_iter)
        {
            cout << encl_iter->first << ": ";
            for (int i = 0; i < encl_iter->second.size(); ++i)
            {
                cout << encl_iter->second[i] << " ";
                if (i == cost_location)
                    cost += encl_iter->second[i];
            }
            cout << endl;
            cout << "---------------------" << endl;
        }
        cout << "成本: " << cost << endl;
    }
    else
        cout << "Some data files may be empty." << endl;
    return 0;
}