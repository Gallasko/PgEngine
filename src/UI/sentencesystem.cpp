#include "sentencesystem.h"

Sentence::Sentence(const std::string& sentence, float scale, FontLoader *font)
{
    this->scale = scale;
    setText(sentence, font);
}

void Sentence::setText(const std::string& sentence, FontLoader *font)
{
    if(sentence != text)
    {
        nbChara = sentence.length();
        letters = new FontLoader::Font* [sentence.length()];

        width = 0;
        for(int i = 0; i < sentence.length(); i++)
        {
            letters[i] = font->getChara(std::string(1, sentence.at(i)));
            width += letters[i]->getWidth() * scale + 1;

            if(letters[i]->getHeight() * scale + letters[i]->getOffset() > height)
                height = letters[i]->getHeight() * scale + letters[i]->getOffset() * scale;
        }

        text = sentence;

        update();
    }
    /*
    if(sentence != text)
    {
        nbChara = sentence.length();
        FontLoader::Font* letter;
        
        modelInfo.nbVertices = 20 * nbChara;
        modelInfo.nbIndices = 6 * nbChara;

        modelInfo.vertices = new float [modelInfo.nbVertices];
        modelInfo.indices = new unsigned int [modelInfo.nbIndices];

        width = 0;
        for(int i = 0; i < sentence.length(); i++)
        {
            letter = font->getChara(std::string(1, sentence.at(i)));
            auto letterModel = letter->getModelInfo();

            modelInfo.vertices[i * 20 + 0]  = letterModel.vertices[0]  + 1.0f * i + 0.05f; modelInfo.vertices[i * 20 + 1]  = letterModel.vertices[1];  modelInfo.vertices[i * 20 + 2]  = letterModel.vertices[2];  modelInfo.vertices[i * 20 + 3]  = letterModel.vertices[3];  modelInfo.vertices[i * 20 + 4]  = letterModel.vertices[4];   
			modelInfo.vertices[i * 20 + 5]  = letterModel.vertices[5]  + 1.0f * i + 0.05f; modelInfo.vertices[i * 20 + 6]  = letterModel.vertices[6];  modelInfo.vertices[i * 20 + 7]  = letterModel.vertices[7];  modelInfo.vertices[i * 20 + 8]  = letterModel.vertices[8];  modelInfo.vertices[i * 20 + 9]  = letterModel.vertices[9];
			modelInfo.vertices[i * 20 + 10] = letterModel.vertices[10] + 1.0f * i + 0.05f; modelInfo.vertices[i * 20 + 11] = letterModel.vertices[11]; modelInfo.vertices[i * 20 + 12] = letterModel.vertices[12]; modelInfo.vertices[i * 20 + 13] = letterModel.vertices[13]; modelInfo.vertices[i * 20 + 14] = letterModel.vertices[14];
			modelInfo.vertices[i * 20 + 15] = letterModel.vertices[15] + 1.0f * i + 0.05f; modelInfo.vertices[i * 20 + 16] = letterModel.vertices[16]; modelInfo.vertices[i * 20 + 17] = letterModel.vertices[17]; modelInfo.vertices[i * 20 + 18] = letterModel.vertices[18]; modelInfo.vertices[i * 20 + 19] = letterModel.vertices[19];

			modelInfo.indices[i * 6 + 0] = 4 * i + 0; modelInfo.indices[i * 6 + 1] = 4 * i + 1; modelInfo.indices[i * 6 + 2] = 4 * i + 2;
			modelInfo.indices[i * 6 + 3] = 4 * i + 1; modelInfo.indices[i * 6 + 4] = 4 * i + 2; modelInfo.indices[i * 6 + 5] = 4 * i + 3;

            width += letter->getWidth() * scale + 1;

            if(letter->getHeight() * scale + letter->getOffset() > height)
                height = letter->getHeight() * scale + letter->getOffset();
        }

        text = sentence;

        update();
    }
    */
}