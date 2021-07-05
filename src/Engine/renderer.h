//TO DO Renderer class

/*
const qreal retinaScale = devicePixelRatio();

    QMatrix4x4 projection;
    QMatrix4x4 view;
    QMatrix4x4 model;
    QMatrix4x4 scale;

    projection.setToIdentity();
    model.setToIdentity();
    scale.setToIdentity();
    scale.scale(QVector3D(2.0f / width(), 2.0f / height(), 0.0f));

    // Text rendering

    defaultShaderProgram->bind();

    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("projection"), projection);
    defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("model"), model);

    //gl scissor for list views 
    //glEnable(GL_SCISSOR_TEST);
    //glScissor(300, 200, 200, 500);

    for(auto texture : ecs.view<TextureComponent>())
    {
        if(texture.visible)
        {
            if(texture.initialised == false)
                texture.generateMesh();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture.texture);

            view.setToIdentity();
            view.translate(QVector3D(-1.0f + 2.0f * (float)(texture.x) / width(), 1.0f + 2.0f * (float)( -texture.y) / height(), 0.0f));

            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("view"), view);
            defaultShaderProgram->setUniformValue(defaultShaderProgram->uniformLocation("scale"), scale);

            texture.VAO->bind();
            glDrawElements(GL_TRIANGLES, texture.modelInfo.nbIndices * 6, GL_UNSIGNED_INT, 0);
        }
    }

    //glDisable(GL_SCISSOR_TEST);

    defaultShaderProgram->release();

*/