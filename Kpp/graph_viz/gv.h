#pragma once

namespace kpp
{
	namespace graph
	{
		struct node
		{
			std::unordered_set<node*> links;

			std::string name,
						label,
						shape,
						fillcolor,
						style;

			void add_link(node* n);
		};

		struct subgraph
		{
			std::vector<node*> nodes;

			std::string name;

			class gv* graph = nullptr;

			void add_node(node* n);
		};

		class gv
		{
		private:

			std::vector<subgraph*> subgraphs;
			std::vector<node*> nodes;
			std::unordered_map<std::string, node*> global_nodes;

			std::string filename,
						name,
						bg_color,
						font_name,
						font_size,
						node_base_name;

		public:

			gv(const std::string& filename, const std::string& name) : filename(filename), name(name) {}

			void set_bg_color(const std::string& val)  { bg_color = val; }
			void set_font_name(const std::string& val) { font_name = val; }
			void set_font_size(const std::string& val) { font_size = val; }

			void build();
			void render(const std::string& title);
			void insert_node(node* n);
			void set_base_name(const std::string& val) { node_base_name = val + "_"; }

			subgraph* create_subgraph(const std::string& name);

			node* create_node(
				const std::string& name,
				const std::string& label,
				const std::string& shape,
				const std::string& fillcolor,
				const std::string& style);

			node* get_node_by_name(const std::string& name);

			void add_node(node* n);
		};
	}
}