#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
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
    BrowserWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUI();
        connectSignals();
    }

private slots:
    void loadUrl() {
        QString url = urlInput->text();
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://" + url; // Ensure URL starts with protocol
        }
        fetchUrl(url);
    }

    void onUrlFetched(QNetworkReply *reply) {
        if (reply->error() == QNetworkReply::NoError) {
            QString htmlContent = QString::fromUtf8(reply->readAll());
            processAndDisplayHtml(htmlContent);
        } else {
            displayError(reply->errorString());
        }
        reply->deleteLater();
    }

private:
    void setupUI() {
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        urlInput = new QLineEdit(this);
        QPushButton *goButton = new QPushButton("Go", this);

        htmlDisplay = new QTextEdit(this);
        htmlDisplay->setReadOnly(true);

        QHBoxLayout *urlLayout = new QHBoxLayout();
        urlLayout->addWidget(urlInput);
        urlLayout->addWidget(goButton);

        layout->addLayout(urlLayout);
        layout->addWidget(htmlDisplay);

        centralWidget->setLayout(layout);
        setCentralWidget(centralWidget);
    }

    void connectSignals() {
        connect(urlInput, &QLineEdit::returnPressed, this, &BrowserWindow::loadUrl);
        connect(goButton, &QPushButton::clicked, this, &BrowserWindow::loadUrl);
    }

    void fetchUrl(const QString &url) {
        QNetworkRequest request(url);
        QNetworkReply *reply = networkManager.get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            onUrlFetched(reply);
        });
    }

    void processAndDisplayHtml(const QString &html) {
        QString processedHtml = applyBasicCss(html);
        processedHtml = executeJavaScript(processedHtml);
        htmlDisplay->setHtml(processedHtml);
    }

    QString applyBasicCss(const QString &html) {
        QString processedHtml = html;

        // Extract CSS from <style> tags
        QRegularExpression styleRegex("<style[^>]*>(.*?)</style>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator matchIterator = styleRegex.globalMatch(html);

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString styleContent = match.captured(1);
            applyCss(styleContent, processedHtml);
            processedHtml.remove(match.capturedStart(0), match.capturedLength(0)); // Remove <style> tag
        }

        return processedHtml;
    }

    void applyCss(const QString &cssContent, QString &html) {
        // Initialize libcss
        css_error error;
        css_stylesheet *sheet;
        error = css_stylesheet_create(CSS_LEVEL_DEFAULT, "UTF-8", NULL, NULL, NULL, NULL, &sheet);
        if (error != CSS_OK) {
            qWarning() << "Failed to create stylesheet";
            return;
        }

        // Parse the CSS content
        error = css_stylesheet_append_data(sheet, cssContent.toUtf8().data(), cssContent.toUtf8().size());
        if (error != CSS_OK) {
            qWarning() << "Failed to append data to stylesheet";
            css_stylesheet_destroy(sheet);
            return;
        }

        error = css_stylesheet_data_done(sheet);
        if (error != CSS_OK) {
            qWarning() << "Failed to finalize stylesheet";
            css_stylesheet_destroy(sheet);
            return;
        }

        // Apply the stylesheet to your HTML content
        // This is a simplified example of applying inline styles
        QRegularExpression pTagRegex("<p[^>]*>");
        QRegularExpressionMatchIterator matchIterator = pTagRegex.globalMatch(html);

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString pTag = match.captured(0);
            QString styledPTag = pTag;
            if (!pTag.contains("style=")) {
                styledPTag.insert(pTag.length() - 1, " style='color:blue;'");
            }
            html.replace(pTag, styledPTag);
        }

        // Clean up
        css_stylesheet_destroy(sheet);
    }

    QString executeJavaScript(const QString &html) {
        QJSEngine jsEngine;
        QRegularExpression scriptRegex("<script[^>]*>(.*?)</script>", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator matchIterator = scriptRegex.globalMatch(html);
        QString modifiedHtml = html;

        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            QString scriptContent = match.captured(1);

            // Evaluate the JavaScript code
            QJSValue result = jsEngine.evaluate(scriptContent);

            if (result.isError()) {
                qWarning() << "Uncaught exception at line"
                           << result.property("lineNumber").toInt() << ":"
                           << result.toString();
            } else {
                // Optionally process the result and modify HTML accordingly
                // Here, simply remove the script tag after execution
                modifiedHtml.remove(match.capturedStart(0), match.capturedLength(0));
            }
        }

        return modifiedHtml;
    }

    void displayError(const QString &error) {
        htmlDisplay->setHtml("<p style='color:red;'><strong>Error:</strong> " + error + "</p>");
    }

    QLineEdit *urlInput;
    QPushButton *goButton;
    QTextEdit *htmlDisplay;
    QNetworkAccessManager networkManager;
    QJSEngine jsEngine;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    BrowserWindow window;
    window.resize(1024, 768);
    window.show();

    return app.exec();
}

#include "main.moc"
