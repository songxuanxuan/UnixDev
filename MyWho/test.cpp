#include <memory>

class Image{};

struct PMImpy {
	std::shared_ptr<Image> bgImage;
	int imageChange;
	
};

class PrettyMenu {
	void changeBackGround();
private:
	std::shared_ptr<PMImpy> pImpy;
};

void PrettyMenu::changeBackGround()
{
	std::shared_ptr<PMImpy> pNew(new PMImpy(*pImpy));
	pNew->bgImage.reset(new Image());
}
