#ifndef LANGUAGERESULTOVERRIDE_H
#define LANGUAGERESULTOVERRIDE_H

#include <string>
#include <map>
#include <memory>
#include <vector>

struct ResultOverride;

typedef std::map<std::string, ResultOverride> languageresultoverridemap_t;
typedef std::shared_ptr<languageresultoverridemap_t> languageresultoverridemap_ptr_t;
typedef std::shared_ptr<const languageresultoverridemap_t> languageresultoverridemapconst_ptr_t;

class Url;

class LanguageResultOverride {
public:
	explicit LanguageResultOverride(const char *filename);
	explicit LanguageResultOverride(languageresultoverridemap_ptr_t languageResultOverrideMap);

	std::string getTitle(const std::string &lang, const Url &url) const;
	std::string getSummary(const std::string &lang, const Url &url) const;

protected:
	bool load();

	void swapLanguageResultOverride(languageresultoverridemapconst_ptr_t resultOverrideMap);
	languageresultoverridemapconst_ptr_t getLanguageResultOverrideMap() const;

	const char *m_filename;
	languageresultoverridemapconst_ptr_t m_languageResultOverrideMap;
	time_t m_lastModifiedTime;
};


#endif //LANGUAGERESULTOVERRIDE_H