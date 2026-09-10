// morphology.hpp —— 二值图像形态学处理（膨胀/腐蚀）面向对象实现
// 设计要点：
//   1) StructuringElement 抽象基类 —— 结构元素可替换（矩形/十字/椭圆…）
//   2) MorphologyOperator 抽象基类 —— 算子可扩展（膨胀/腐蚀…）
//   3) BinaryImage —— 二值图像容器（0/1），支持打印与 PGM 导出
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <ostream>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace morph {

// ---------- 点偏移 ----------
struct Offset {
    int dr = 0;   // 行偏移
    int dc = 0;   // 列偏移
};

// ===========================================================================
// 结构元素（Structuring Element）—— 抽象基类，可替换
// ===========================================================================
class StructuringElement {
public:
    virtual ~StructuringElement() = default;

    // 该结构元素覆盖的所有偏移（相对于锚点 (0,0)）
    virtual std::vector<Offset> offsets() const = 0;

    // 结构元素名字（用于日志/测试输出）
    virtual std::string name() const = 0;

    // 锚点偏移：整体平移结构元素，可用于"定向膨胀"（如只向右下扩展）
    void setAnchor(int dr, int dc) { anchor_r_ = dr; anchor_c_ = dc; }
    int  anchorR() const { return anchor_r_; }
    int  anchorC() const { return anchor_c_; }

    // 相对锚点平移后的实际偏移
    std::vector<Offset> shiftedOffsets() const {
        std::vector<Offset> out;
        for (const auto& o : offsets()) {
            out.push_back(Offset{o.dr - anchor_r_, o.dc - anchor_c_});
        }
        return out;
    }

    // 覆盖范围（用于计算输出尺寸/边界）
    void bounds(int& min_r, int& max_r, int& min_c, int& max_c) const {
        min_r = min_c = 0;
        max_r = max_c = 0;
        bool first = true;
        for (const auto& o : shiftedOffsets()) {
            if (first) { min_r = max_r = o.dr; min_c = max_c = o.dc; first = false; }
            min_r = std::min(min_r, o.dr); max_r = std::max(max_r, o.dr);
            min_c = std::min(min_c, o.dc); max_c = std::max(max_c, o.dc);
        }
    }

private:
    int anchor_r_ = 0;
    int anchor_c_ = 0;
};

// ---------- 具体结构元素 ----------

// 矩形（W×H 全 1）—— 最常用的方形结构元素
class RectElement : public StructuringElement {
public:
    RectElement(int w, int h) : w_(w), h_(h) {
        if (w <= 0 || h <= 0) throw std::invalid_argument("RectElement 尺寸必须为正");
    }
    std::vector<Offset> offsets() const override {
        std::vector<Offset> v;
        for (int r = -(h_ / 2); r <= (h_ - 1) / 2; ++r)
            for (int c = -(w_ / 2); c <= (w_ - 1) / 2; ++c)
                v.push_back(Offset{r, c});
        return v;
    }
    std::string name() const override {
        return "Rect(" + std::to_string(w_) + "x" + std::to_string(h_) + ")";
    }
private:
    int w_, h_;
};

// 十字形（半径 r）
class CrossElement : public StructuringElement {
public:
    explicit CrossElement(int r) : r_(r) {
        if (r < 0) throw std::invalid_argument("CrossElement 半径不能为负");
    }
    std::vector<Offset> offsets() const override {
        std::vector<Offset> v;
        for (int d = -r_; d <= r_; ++d) {
            v.push_back(Offset{0, d});
            if (d != 0) v.push_back(Offset{d, 0});
        }
        return v;
    }
    std::string name() const override {
        return "Cross(r=" + std::to_string(r_) + ")";
    }
private:
    int r_;
};

// 椭圆/圆盘近似（半轴 rx, ry）—— 用圆形结构元素可得到"圆角/圆形外扩"的膨胀结果
class EllipseElement : public StructuringElement {
public:
    EllipseElement(int rx, int ry) : rx_(rx), ry_(ry) {
        if (rx < 0 || ry < 0) throw std::invalid_argument("EllipseElement 半径不能为负");
    }
    std::vector<Offset> offsets() const override {
        std::vector<Offset> v;
        const double a = rx_, b = ry_;
        for (int r = -ry_; r <= ry_; ++r) {
            for (int c = -rx_; c <= rx_; ++c) {
                if (a == 0 || b == 0) {
                    if (r == 0 && c == 0) v.push_back(Offset{r, c});
                    continue;
                }
                const double norm = (double)(c * c) / (a * a) + (double)(r * r) / (b * b);
                if (norm <= 1.0 + 1e-9) v.push_back(Offset{r, c});
            }
        }
        return v;
    }
    std::string name() const override {
        return "Ellipse(rx=" + std::to_string(rx_) + ",ry=" + std::to_string(ry_) + ")";
    }
private:
    int rx_, ry_;
};

// ===========================================================================
// 二值图像
// ===========================================================================
class BinaryImage {
public:
    BinaryImage() = default;
    BinaryImage(int rows, int cols, uint8_t value = 0)
        : rows_(rows), cols_(cols), data_((size_t)rows * cols, value ? 1 : 0) {
        if (rows <= 0 || cols <= 0) throw std::invalid_argument("图像尺寸必须为正");
    }

    static BinaryImage fromRows(const std::vector<std::vector<int>>& rows) {
        if (rows.empty() || rows[0].empty()) throw std::invalid_argument("空图像");
        BinaryImage img((int)rows.size(), (int)rows[0].size());
        for (int r = 0; r < img.rows(); ++r)
            for (int c = 0; c < img.cols(); ++c)
                img.set(r, c, rows[r][c] ? 1 : 0);
        return img;
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    uint8_t at(int r, int c) const {
        if (r < 0 || c < 0 || r >= rows_ || c >= cols_) return 0;   // 越界按背景处理
        return data_[(size_t)r * cols_ + c];
    }
    void set(int r, int c, uint8_t v) {
        if (r < 0 || c < 0 || r >= rows_ || c >= cols_) return;
        data_[(size_t)r * cols_ + c] = v ? 1 : 0;
    }
    int count() const {
        int n = 0;
        for (auto v : data_) n += v ? 1 : 0;
        return n;
    }
    bool operator==(const BinaryImage& o) const {
        return rows_ == o.rows_ && cols_ == o.cols_ && data_ == o.data_;
    }
    bool operator!=(const BinaryImage& o) const { return !(*this == o); }

    // 文本打印（1 = '#', 0 = '.'），便于终端查看与文档配图
    void print(std::ostream& os, char fg = '#', char bg = '.') const {
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) os << (at(r, c) ? fg : bg) << ' ';
            os << '\n';
        }
    }
    // 数字矩阵打印（与任务书示例一致的 0/1 展示）
    void printNumbers(std::ostream& os) const {
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) os << (int)at(r, c) << ' ';
            os << '\n';
        }
    }
    // 导出 PGM（P2 文本格式，可用任意看图软件/PS 打开，作为"选做：可视化"）
    void savePGM(const std::string& path) const {
        std::ofstream f(path);
        if (!f) throw std::runtime_error("无法写入 " + path);
        f << "P2\n" << cols_ << ' ' << rows_ << "\n255\n";
        for (int r = 0; r < rows_; ++r) {
            for (int c = 0; c < cols_; ++c) f << (at(r, c) ? 255 : 0) << ' ';
            f << '\n';
        }
    }
    // 供测试/绘图使用的原始行向量
    std::vector<std::vector<int>> toRows() const {
        std::vector<std::vector<int>> rows(rows_, std::vector<int>(cols_, 0));
        for (int r = 0; r < rows_; ++r)
            for (int c = 0; c < cols_; ++c) rows[r][c] = at(r, c);
        return rows;
    }

private:
    int rows_ = 0, cols_ = 0;
    std::vector<uint8_t> data_;
};

// ===========================================================================
// 形态学算子 —— 抽象基类（膨胀/腐蚀都实现同一接口，可扩展其他算子）
// ===========================================================================
class MorphologyOperator {
public:
    virtual ~MorphologyOperator() = default;
    virtual std::string name() const = 0;
    virtual BinaryImage apply(const BinaryImage& src, const StructuringElement& se) const = 0;
};

// 膨胀：输出像素为 1 ⇔ 结构元素覆盖范围内**存在**前景像素（∃）
// Dst(x) = 1  if  ∃ s ∈ SE : Src(x - s) == 1
// 越界按背景(0)处理 —— 因此图像边界不会被"凭空撑大"
class DilateOp : public MorphologyOperator {
public:
    std::string name() const override { return "Dilation"; }
    BinaryImage apply(const BinaryImage& src, const StructuringElement& se) const override {
        BinaryImage dst(src.rows(), src.cols());
        const auto offs = se.shiftedOffsets();
        for (int r = 0; r < src.rows(); ++r) {
            for (int c = 0; c < src.cols(); ++c) {
                for (const auto& o : offs) {
                    if (src.at(r - o.dr, c - o.dc)) { dst.set(r, c, 1); break; }
                }
            }
        }
        return dst;
    }
};

// 腐蚀：输出像素为 1 ⇔ 结构元素覆盖范围内**全部**为前景像素（∀）
// Dst(x) = 1  if  ∀ s ∈ SE : Src(x + s) == 1
class ErodeOp : public MorphologyOperator {
public:
    std::string name() const override { return "Erosion"; }
    BinaryImage apply(const BinaryImage& src, const StructuringElement& se) const override {
        BinaryImage dst(src.rows(), src.cols());
        const auto offs = se.shiftedOffsets();
        for (int r = 0; r < src.rows(); ++r) {
            for (int c = 0; c < src.cols(); ++c) {
                if (!src.at(r, c)) continue;              // 背景保持背景
                bool all = true;
                for (const auto& o : offs) {
                    if (!src.at(r + o.dr, c + o.dc)) { all = false; break; }
                }
                if (all) dst.set(r, c, 1);
            }
        }
        return dst;
    }
};

// 方向性膨胀：把锚点平移，使膨胀只朝某个方向扩展（用于自定义形状）
class DirectionalDilateOp : public MorphologyOperator {
public:
    DirectionalDilateOp(int anchor_r, int anchor_c) : ar_(anchor_r), ac_(anchor_c) {}
    std::string name() const override { return "DirectionalDilation"; }
    BinaryImage apply(const BinaryImage& src, const StructuringElement& se_in) const override {
        StructuringElement* p = const_cast<StructuringElement*>(&se_in);
        p->setAnchor(-ar_, -ac_);      // 锚点取反 → 使膨胀朝 (ar_, ac_) 指定的方向扩展
        return DilateOp().apply(src, *p);
    }
private:
    int ar_, ac_;
};

}  // namespace morph
