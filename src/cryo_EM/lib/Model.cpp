#include "Model.h"

Model::Model() {
}

Model::~Model() {
    if(m_program!=nullptr)
        delete m_program;
    for (auto& texture : m_textures) {
        delete texture;
    }
}

void Model::setTexture(QOpenGLTexture *texture, int index) {
    if (index == -1) {
        if (m_textures.isEmpty()) {
            index = 0;
        } else {
            index = m_textures.lastKey() + 1;  // 避免创建一个list导致的消耗
        }
    }
    m_textures.insert(index, texture);
}


QMatrix4x4 Model::model() {
    QMatrix4x4 _mat;
	_mat.setToIdentity();
	_mat.translate(m_pos);
	_mat.rotate(m_rotate.z(), 0, 0, 1);
	_mat.rotate(m_rotate.y(), 0, 1, 0);
	_mat.rotate(m_rotate.x(), 1, 0, 0);
	_mat.scale(m_scale);
	return _mat;//先缩放再绕x轴旋转、再绕y轴旋转、再绕z轴旋转、再平移
}

void Model::init() {
}

void Model::update() {
}

void Model::paint() {
}
