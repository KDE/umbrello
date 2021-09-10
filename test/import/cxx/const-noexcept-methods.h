class ConstNoexceptMethodClass {
    bool ConstMethod(QString &param) const;
    bool NoexceptMethod(QString &param) noexcept;
    bool ConstNoexceptMethod(QString &param) const noexcept { return true }
};
