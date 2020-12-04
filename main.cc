#include <memory>
#include <iostream>
#include <ctime>

#include <QtCore>
#include <QApplication>
#include <QMainWindow>
#include <QRandomGenerator>
#include <QThread>

#include "ui_main.h"

struct Part {
	std::string source;
	int collisions;
};

struct  CSMD {

	static bool hasJam;
	static QRandomGenerator *rmd;

	QList<Part> parts;	

	static bool chance() {
		return rmd->securelySeeded().bounded(0, 9) < 6; 
	}

	static void delay(int a) {
		auto c = rmd->securelySeeded().bounded(0, pow(2, a));
		QThread::msleep(c);	
	}

	static Part doCsmd(const std::string &source) {
		int channelAttempts = 0;
		while (!CSMD::chance()) 
			CSMD::delay(channelAttempts++); 
		
		int attempts = 0;
		while (true) {
			if (!CSMD::chance()) break;
			CSMD::hasJam = true;
			if (++attempts == 10) {
				return Part{source, 10};
			}
		}

		return Part{source, attempts}; 
	}

	static CSMD streamCsmd(int frameSize, std::string source) {
		if (source.size() % frameSize != 0)
			source += std::string(source.size() % frameSize, '0');
		QList<Part> ret;
		for (int i = 0; i < source.size(); i+=frameSize) {
			ret.append(CSMD::doCsmd(source.substr(i, frameSize)));
		}
		return CSMD{ret};
	}
};

QRandomGenerator *CSMD::rmd = new QRandomGenerator;
bool CSMD::hasJam = false;

class MainWindow : public QMainWindow {
	std::unique_ptr<Ui::MainWindow> ui;

public:
	explicit MainWindow(QWidget *parent = nullptr): QMainWindow(parent) {
		ui = std::make_unique<Ui::MainWindow>();
		ui->setupUi(this);
		connect(ui->send, &QPushButton::clicked, [&]() { onSendClicked(); });

		setResult(nullptr);
	}

	void setResult(CSMD *res) {
		if (res== nullptr) {
			ui->input->clear();
			ui->out->clear();
			ui->status->clear();
		} else {
			int index = 0 ;
			QString result;
			for (const auto &part : res->parts) {
				result += "Part #" + QString::number(index++) + "<br/>";
				if (part.collisions >= 10) {
					result += "10 collisions exceeded<br>";
				} else {
					result += QString::fromStdString(part.source);
					result += "|" + QString(part.collisions, '*') + "<br/>";
				}
			}
			ui->status->setHtml(result);
		}
	}

	void onSendClicked() {
		auto text = ui->input->toPlainText();
		if (!text.isEmpty()) {
			auto res = CSMD::streamCsmd(2, text.toStdString());
			setResult(&res);
		}
	}

};

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  MainWindow window;
  window.show();
  return QApplication::exec();
}
