
#include <vector>
#include <string>


class ServerObjectLint
{
	public:
		static void run(const std::vector<std::string> & templateList);
		static void install();

	private:
		ServerObjectLint();
		ServerObjectLint(const ServerObjectLint&);
		ServerObjectLint& operator= (const ServerObjectLint&);
};
