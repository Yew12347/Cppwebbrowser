#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QString>
#include <QRegularExpression>
#include <QJSEngine>
#include <libcss/libcss.h>

class BrowserWindow : public QMainWindow {
    Q_OBJECT
public:
    BrowserWindow(QWidget *parent = nullptr);

private slots:
    void loadUrl();
    void onUrlFetched(QNetworkReply *reply);

private:
    void setupUI();
    void connectSignals();
    void fetchUrl(const QString &url);
    void processAndDisplayHtml(const QString &html);
    QString applyBasicCss(const QString &html);
    void applyCss(const QString &cssContent, QString &html);
    QString executeJavaScript(const QString &html);
    void displayError(const QString &error);

    QLineEdit *urlInput;
    QPushButton *goButton;
    QTextEdit *htmlDisplay;
    QNetworkAccessManager networkManager;
    QJSEngine jsEngine;
};

#endif // BROWSERWINDOW_H
