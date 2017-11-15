"""Test the wikitablescrape script on four articles."""

import os
import shutil

import wikitablescrape

# Delete previous output folder if it exists, then create a new one
try:
    shutil.rmtree('output')
except FileNotFoundError:
    pass

wikitablescrape.scrape(
    url="https://en.wikipedia.org/wiki/List_of_mountains_by_elevation",
    output_name="mountains"
)

wikitablescrape.scrape(
    url="https://en.wikipedia.org/wiki/List_of_volcanoes_by_elevation",
    output_name="volcanoes"
)

wikitablescrape.scrape(
    url="https://en.wikipedia.org/wiki/List_of_current_NBA_team_rosters",
    output_name="123"
)

wikitablescrape.scrape(
    url="https://en.wikipedia.org/wiki/List_of_National_Basketball_Association_career_scoring_leaders",
    output_name="nba"
)



wikitablescrape.scrape(
    url="https://en.wikipedia.org/wiki/List_of_highest-grossing_films",
    output_name="films"
)

# Move all CSV folders into a single 'output' folder
os.makedirs('output')
shutil.move('./outputTables/123', './output')

shutil.move('./outputTables/mountains', './output')
shutil.move('./outputTables/volcanoes', './output')
shutil.move('./outputTables/nba', './output')
shutil.move('./outputTables/films', './output')

