import re

import requests
from bs4 import BeautifulSoup as bs


class NewsCrawler:
    def __init__(self, subject, year, month):
        self.subject = subject
        self.year = year
        self.month = month

        self.wordCounter = {}

        self.homeURL = "https://www.ynet.co.il"

        # link = self.getLinkFromSubject()

        articles = self.getArticlesURLs("https://www.ynet.co.il/home/0,7340,L-4269-723-56-201801-1,00.html")

        for article in articles:
            self.addWordsFromArticle(article)

        navot = 'kanas'

    def getLinkFromSubject(self):
        archive_html = requests.get("https://www.ynet.co.il/home/0,7340,L-4269,00.html").text

        soup = bs(archive_html)

        links = soup.find_all('a', text=re.compile("^" + self.subject + "$"), attrs={'class': 'CSH'})

        return [self.homeURL + link.attrs['href'] for link in links][0]

    def getArticlesPageFromYearAndMonth(self, link):
        pass

    def getArticlesURLs(self, link):
        articlesPageHtml = requests.get(link).text

        soup = bs(articlesPageHtml)

        links = soup.find_all('a', attrs={'class': 'smallheader'})

        return [self.homeURL + link.attrs['href'] for link in links]

    def addToBigDic(self, word):
        if word in self.wordCounter:
            self.wordCounter[word] += 1
        else:
            self.wordCounter[word] = 1




if __name__ == '__main__':
    newsCrawler = NewsCrawler("כדורגל עולמי", 2018, "ינואר")
