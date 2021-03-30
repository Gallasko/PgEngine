#ifndef BLOCKLOADER_H
#define BLOCKLOADER_H

#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

enum class TileType 
{
    BLANK,
    HOUSE,
    SHOP,
    ROAD,
	MISC
};

class TilesLoader : protected QOpenGLFunctions
{
public:
	class Tiles : protected QOpenGLFunctions
	{
	friend class TilesLoader;
	public:
		Tiles();
		~Tiles();

		inline int getId() const { return id; }
		inline std::string getName() const { return name; }
		inline QOpenGLVertexArrayObject* getMesh() const { return VAO; }

		inline bool operator==(const std::string& rhs) const { return name == rhs; } 
		inline bool operator==(const TileType& rhs) const { return tileType == rhs; }

	protected:
		inline void setId(int id) { this->id = id; }
		inline void setName(std::string name) { this->name = name; }
		void setMesh(int textureId); 
		void setType(std::string type);

	private:
		int id = 0;
		std::string name = "";
		TileType tileType;

		QOpenGLVertexArrayObject *VAO;
		QOpenGLBuffer *VBO;
		QOpenGLBuffer *EBO;
	};

	TilesLoader(std::string tilesFolder);
	~TilesLoader();

	TilesLoader::Tiles* getTile(int id) const;
	TilesLoader::Tiles* getTile(std::string tileName) const;

private:
	std::vector<TilesLoader::Tiles* > tilesList;
	std::map<std::string, int> tilesDict;
	int nbTilesId;
};

#endif