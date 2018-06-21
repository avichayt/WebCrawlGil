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

        #todo remove comment

        # link = self.getLinkFromSubject()

        # articles = self.getArticlesURLs("https://www.ynet.co.il/home/0,7340,L-4269-723-56-201801-1,00.html")
        #
        # for article in articles:
        #     self.addWordsFromArticle(article)

        self.addWordsFromArticle("https://www.ynet.co.il/articles/0,7340,L-5064774,00.html")

        navot = 'kanas'

    def getLinkFromSubject(self):
        archive_html = requests.get("https://www.ynet.co.il/home/0,7340,L-4269,00.html").text

        soup = bs(archive_html)

        links = soup.find_all('a', text=re.compile("^" + self.subject + "$"), attrs={'class': 'CSH'})

        return [self.homeURL + link.attrs['href'] for link in links][0]

    def getArticlesPageFromYearAndMonth(self, link):
        archive_html = requests.get(link).text
        soup = bs(archive_html)
        tds = soup.find_all('td', {'class':'classMainTitle'})

        year_td = [td for td in tds if td.find_all('b')[0].text == "שנת {}".format(str(self.year))][0]
        articles_link = year_td.parent.find_all('td')[self.month].find_all('a')[0].attrs['href']
        return self.homeURL+ articles_link


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

    def addLineToBigDic(self, line):

        for word in line.split():
            self.addToBigDic(word)

    def addWordsFromArticle(self, article):
        articleHTML = requests.get(article).text
        soup = bs(articleHTML)

        main_title_text = soup.find_all('div', attrs={'class': 'art_header_title'})[0].contents[0]

        sub_title_text = soup.find_all('div', attrs={'class': 'art_header_sub_title'})[0].contents[0]



        pass


if __name__ == '__main__':
    newsCrawler = NewsCrawler("חדשות", 2017, 1)
    print(newsCrawler.getArticlesPageFromYearAndMonth(newsCrawler.getLinkFromSubject()))
