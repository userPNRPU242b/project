#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QDialog>
#include <QSqlDatabase>

// Предварительное объявление классов Qt (уменьшаем зависимости)
class QTableWidgetItem;
class QPushButton;
namespace Ui {
class archive;
}

class archive : public QDialog
{
    Q_OBJECT

public:
    // Конструктор с параметрами
    explicit archive(QWidget *parent = nullptr, const QString &username = QString());
    ~archive();

private slots:
    // Слоты для обработки событий
    void saveAllChanges();                          // Сохранение всех изменений
    void on_tableWidget_cellChanged(int row, int column); // Изменение ячейки таблицы
    void on_pushButton_clicked();                   // Обработка кнопки возврата

private:
    // Внутренние методы
    void setupWindowProperties();    // Настройка свойств окна
    void applyStyles();              // Применение стилей интерфейса
    void setupTable();               // Настройка таблицы данных
    void setupSaveButton();          // Настройка кнопки сохранения
    void loadDataFromDatabase();     // Загрузка данных из БД
    void addTableRow(const QSqlQuery& query); // Добавление строки в таблицу
    void addNonEditableItem(int row, int column, const QString& text, const QVariant& userData = QVariant());
    bool saveRowChanges(int row, QSqlDatabase& db); // Сохранение изменений строки
    void finalizeTransaction(QSqlDatabase& db, bool hasErrors); // Завершение транзакции
    void highlightSavedChanges();    // Подсветка сохраненных изменений
    void highlightSavedRow(int row); // Подсветка сохраненной строки

    // Вспомогательные методы для сообщений
    void showError(const QString& message);
    void showWarning(const QString& message);
    void showInfo(const QString& message);

    // Члены класса
    Ui::archive *ui;                // Указатель на UI-форму
    QString currentUsername;         // Текущий пользователь
    QPushButton* saveButton;         // Кнопка сохранения (для управления из кода)
};

#endif // ARCHIVE_H
