#ifndef FONTLOADER_H
#define FONTLOADER_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <vector>
#include <string>
#include <unordered_map>

#include "..\constant.h"

namespace pg
{
	class FontLoader : protected QOpenGLFunctions
	{
	public:
		class Font : protected QOpenGLFunctions
		{
		friend class FontLoader;
		public:
			Font();
			~Font();

			inline int getId() const { return id; }
			inline std::string getName() const { return name; }
			inline QOpenGLVertexArrayObject* getMesh() const { return VAO; }
			
			inline unsigned int getWidth() const { return width; }
			inline unsigned int getHeight() const { return height; }
			inline unsigned int getOffset() const { return yOffset; }

			inline constant::SquareInfo getModelInfo() const { return modelInfo; }

		protected:
			inline void setId(int id) { this->id = id; }
			inline void setWidth(unsigned int w) { this->width = w; }
			inline void setHeight(unsigned int h) { this->height = h; }
			inline void setOffset(unsigned int offset) { this->yOffset = offset; }
			inline void setName(const std::string& name) { this->name = name; }
			void setMesh(unsigned int xPos, unsigned int yPos, unsigned int atlasWidth, unsigned int atlasHeight); 

		private:
			int id = 0;
			std::string name = "";
			unsigned int width = 0;
			unsigned int height = 0;
			unsigned int yOffset = 0;

			constant::SquareInfo modelInfo;

			QOpenGLVertexArrayObject *VAO;
			QOpenGLBuffer *VBO;
			QOpenGLBuffer *EBO;
		};

		FontLoader(const std::string& fontFile);
		~FontLoader();

		FontLoader::Font* getChara(int id) const;
		FontLoader::Font* getChara(const std::string& charaName) const;

		inline unsigned int getAtlasWidth() const { return atlasWidth; }
		inline unsigned int getAtlasHeight() const { return atlasHeight; }

	private:
		std::vector<FontLoader::Font* > charaList;
		std::unordered_map<std::string, int> charaDict;
		int nbCharaId;

		unsigned int atlasWidth = 0;
		unsigned int atlasHeight = 0;
		int opacity[3] = { -1, -1, -1 };
	};
}

#endif