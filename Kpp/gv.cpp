#include <defs.h>

#include "gv.h"

using namespace kpp;
using namespace graph;

void gv::build()
{
	auto file = std::ofstream(filename, std::ios_base::trunc);
	if (!file)
		return;

	file << "digraph " << name << " {" << std::endl;

	file << dbg::format(R"(graph [fontname = "%s", fontsize = %s])", font_name.c_str(), font_size.c_str()) << std::endl;
	file << dbg::format(R"(node [fontname = "%s", fontsize = %s])", font_name.c_str(), font_size.c_str()) << std::endl;
	file << dbg::format(R"(edge [fontname = "%s", fontsize = %s])", font_name.c_str(), font_size.c_str()) << std::endl;
	file << dbg::format(R"(bgcolor = "%s")", bg_color.c_str()) << std::endl;
	
	int cluster_id = 0;

	for (auto&& subgraph : subgraphs)
	{
		file << dbg::format(R"(subgraph cluster_%i {)", cluster_id++) << std::endl;
		file << dbg::format(R"(label = "%s")", subgraph->name.c_str()) << std::endl;

		for (auto&& node : subgraph->nodes)
			file << dbg::format(R"(node [label = "%s", style = %s, shape = "%s", fillcolor = "%s"] %s)", node->label.c_str(), node->style.c_str(), node->shape.c_str(), node->fillcolor.c_str(), node->name.c_str()) << std::endl;

		file << "}" << std::endl;
	}

	for (auto&& node : nodes)
	{
		file << dbg::format(R"(node [label = "%s", style = %s, shape = "%s", fillcolor = "%s"] %s)", node->label.c_str(), node->style.c_str(), node->shape.c_str(), node->fillcolor.c_str(), node->name.c_str()) << std::endl;

		for (auto&& link : node->links)
			file << dbg::format(R"(%s -> %s)", node->name.c_str(), link->name.c_str()) << std::endl;
	}

	for (auto&& subgraph : subgraphs)
		for (auto&& node : subgraph->nodes)
			for (auto&& link : node->links)
				file << dbg::format(R"(%s -> %s)", node->name.c_str(), link->name.c_str()) << std::endl;

	file << "}" << std::endl;
}

void gv::render(const std::string& title)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	si.cb = sizeof(si);

	if (!CreateProcessA("C:\\Program Files\\Graphviz 2.44.1\\bin\\dot.exe", (char*)("dot.exe -Tpng -O " + filename).c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
		return;

	WaitForSingleObject(pi.hProcess, INFINITE);

	if (auto image = cv::imread(filename + ".png"); !image.empty())
	{
		int win_w = image.cols,
			win_h = std::clamp(image.rows, 0, 1200);

		cv::namedWindow(title, cv::WINDOW_NORMAL);
		cv::resizeWindow(title, win_w, win_h);
		cv::moveWindow(title, GetSystemMetrics(SM_CXSCREEN) / 2 - win_w / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - win_h / 2);

		if (win_h >= image.rows) win_h = image.rows - 1;
		if (win_w >= image.cols) win_w = image.cols - 1;

		int scroll_h = 0,
			scroll_w = 0;

		cv::createTrackbar("Width", title, &scroll_w, (image.cols - win_w));
		cv::createTrackbar("Height", title, &scroll_h, (image.rows - win_h));

		do cv::imshow(title, image(cv::Rect(scroll_w, scroll_h, win_w, win_h)));
		while (cv::waitKey(1) != VK_ESCAPE && cv::getWindowProperty(title, cv::WND_PROP_VISIBLE));

		cv::destroyWindow(title);
	}
}

void gv::insert_node(node* n)
{
	global_nodes.insert({ n->name, n });
}

subgraph* gv::create_subgraph(const std::string& name)
{
	auto s = new subgraph();

	s->name = name;
	s->graph = this;

	subgraphs.push_back(s);

	return s;
}

node* gv::create_node(
	const std::string& name,
	const std::string& label,
	const std::string& shape,
	const std::string& fillcolor,
	const std::string& style)
{
	if (auto it = global_nodes.find(name); it != global_nodes.end())
		return nullptr;
	
	auto n = new node();

	n->name = node_base_name + name;
	n->label = label;
	n->shape = shape;
	n->fillcolor = fillcolor;
	n->style = style;

	insert_node(n);

	return n;
}

node* gv::get_node_by_name(const std::string& name)
{
	auto it = global_nodes.find(node_base_name + name);
	return (it != global_nodes.end() ? it->second : nullptr);
}

void gv::add_node(node* n)
{
	nodes.push_back(n);
}

void subgraph::add_node(node* n)
{
	nodes.push_back(n);
}

void node::add_link(node* n)
{
	if (links.find(n) == links.end())
		links.insert(n);
}