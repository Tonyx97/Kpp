#pragma once

namespace kpp
{
	namespace util
	{
		namespace stl
		{
			template <typename Tx, typename Ty>
			class zip
			{
			private:

				Tx v1;
				Ty v2;

			public:

				struct it_tuple
				{
					Tx::iterator it_v1;
					Ty::iterator it_v2;

					auto operator != (it_tuple t) { return (it_v1 != t.it_v1 && it_v2 != t.it_v2); }
				
					it_tuple& operator ++ ()	  { ++it_v1; ++it_v2; return *this; }
					it_tuple& operator * ()		  { return *this; }
				};

				zip(const Tx& v1, const Ty& v2) : v1(v1), v2(v2) {}

				it_tuple begin() { return { v1.begin(), v2.begin() }; }
				it_tuple end()   { return { v1.end(), v2.end() }; }
			};
			
			template <typename Tx, typename Ty>
			class zip_next
			{
			private:

				Tx v1;
				Ty v2;

			public:

				struct it_tuple
				{
					Tx::iterator it_v1, it_v1_next, v1_end;
					Ty::iterator it_v2, it_v2_next, v2_end;

					auto operator != (it_tuple t) { return (it_v1 != t.it_v1 && it_v2 != t.it_v2); }
				
					it_tuple& operator ++ ()
					{
						++it_v1; ++it_v2;

						if (it_v1_next != v1_end) ++it_v1_next;
						if (it_v2_next != v2_end) ++it_v2_next;

						return *this;
					}

					it_tuple& operator * ()		  { return *this; }
				};

				zip_next(const Tx& v1, const Ty& v2) : v1(v1), v2(v2) {}

				it_tuple begin() { return { v1.begin(), v1.begin() + 1, v1.end(), v2.begin(), v2.begin() + 1, v2.end() }; }
				it_tuple end()   { return { v1.end(),   v1.end(),		v1.end(), v2.end(),   v2.end(),		  v2.end() }; }

				operator bool()  { return (!v1.empty() && !v2.empty()); }
			};
		}

		namespace winapi
		{
			inline bool get_PATH(std::vector<std::string>& paths)
			{
				HKEY key = nullptr;

				if (RegOpenKey(HKEY_CURRENT_USER, L"Environment\\", &key) != ERROR_SUCCESS)
					return false;

				std::string path_list; path_list.resize(8192);

				DWORD size = 0,
					  type = REG_EXPAND_SZ;

				auto res = RegQueryValueExA(key, "Path", nullptr, nullptr, (BYTE*)path_list.data(), &size);
				while (res == ERROR_MORE_DATA)
				{
					path_list.resize(size);
					res = RegQueryValueExA(key, "Path", nullptr, nullptr, (BYTE*)path_list.data(), &size);
				}

				if (res != ERROR_SUCCESS)
					return false;

				std::stringstream ss(path_list);

				std::string path;

				while (std::getline(ss, path, ';'))
					paths.push_back(path);

				return true;
			}
		}
	}
}