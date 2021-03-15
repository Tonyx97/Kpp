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
	file << "graph [dpi = 100]" << std::endl;

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
	std::vector<std::string> paths;

	if (!util::winapi::get_PATH(paths))
		return;

	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));

	si.cb = sizeof(si);

	bool graphviz_running = false;

	for (const auto& path : paths)
		if (graphviz_running = CreateProcessA((path + "\\dot.exe").c_str(), (char*)("dot.exe -Tpng -O " + filename).c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi))
			break;

	if (!graphviz_running)
		return;

	if (WaitForSingleObject(pi.hProcess, 5000) == WAIT_TIMEOUT)
		return;

	if (auto img = cv::imread(filename + ".png"); !img.empty())
	{
		int win_w = std::clamp(img.cols, 0, 1600),
			win_h = std::clamp(img.rows, 0, 1200);

		cv::namedWindow(title, cv::WINDOW_NORMAL);
		cv::resizeWindow(title, win_w, win_h);
		cv::moveWindow(title, GetSystemMetrics(SM_CXSCREEN) / 2 - win_w / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - win_h / 2);

		if (win_h >= img.rows) win_h = img.rows;
		if (win_w >= img.cols) win_w = img.cols;

		int scroll_h = 0,
			scroll_w = 0,
			wheel_delta = 0;

		cv::setMouseCallback(title, [](int event, int x, int y, int flags, void* userdata)
		{
			*(int*)userdata = (event == cv::EVENT_MOUSEWHEEL ? int(float(-cv::getMouseWheelDelta(flags)) * 0.5f) : 0);
		}, &wheel_delta);

		do
		{
			cv::imshow(title, img(cv::Rect(scroll_w, scroll_h, win_w, win_h)));

			if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
				scroll_w = std::clamp(scroll_w + wheel_delta, 0, img.cols - win_w);
			else scroll_h = std::clamp(scroll_h + wheel_delta, 0, img.rows - win_h);

			wheel_delta = 0;
		}
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