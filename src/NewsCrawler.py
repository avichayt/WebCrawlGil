import re

import requests
from bs4 import BeautifulSoup as bs


class NewsCrawler:
    def __init__(self, subject, year, month):
        self.subject = subject
        self.year = year
        self.month = month

        self.homeURL = "https://www.ynet.co.il"

        links = self.getLinksFromSubject()



        navot = 'kanas'

    def getLinksFromSubject(self):
        archive_html = requests.get("https://www.ynet.co.il/home/0,7340,L-4269,00.html").text

        soup = bs(archive_html)

        links = soup.find_all('a', text=re.compile("^" + self.subject + "$"), attrs={'class': 'CSH'})

        return [self.homeURL + link.attrs['href'] for link in links]

    def getArticlesPageFromYearAndMonth(self, link):
        pass

    def getArticlesURLs(self):
        pass

if __name__ == '__main__':
    newsCrawler = NewsCrawler("חדשות", 2018, "ינואר")
