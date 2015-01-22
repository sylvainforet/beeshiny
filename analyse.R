#rm(list=ls())
setwd("/Users/u5305887/Movies/cam2/")
load(".RData")
save.image()
data12 <- read.csv("12.csv", header=T)
data13 <- read.csv("13.csv", header=T)
data14 <- read.csv("14.csv", header=T)
data15 <- read.csv("14.csv", header=T)

# calculate time bees are tracked
split_data <- split(data12[3], data12$BeeID)
time_tracked <- sapply(split_data, function(x) max(x) - min(x))
library(ggplot2)
t <- qplot(time_tracked, binwidth = 100)
t + labs(title = "Length of time bees were tracked for", x="Number of Frames Tracked", y="Number of bees")

# calculate how many bees on a frame at same time
split_data_by_frame <- split(data12, data12$frame)
number_bees_found <- sapply(split_data_by_frame, nrow)
d <- qplot(number_bees_found, binwidth = 1)
d + labs(title = "Number of bees found on frame at same time", x="Number of bees", y="Number of frames")






View(split_data_by_frame[[3]])
nrow(split_data_by_frame[[3]])

mean(time_tracked)

split_data[[2]]

split_data[[2]]
names(split_data$"1")



m <- sapply(split_data, max)
sapply(split_data$fram, min)


tapply(split_data, frame, mean)
names(split_data)
max(split_data$"0"$frame)


sapply(split_data, min)


queen <- subset(data, data$Tag == 3)
#View(queen)
#length(queen$frame)
#unique(length(queen$frame))
table(data$BeeID)
length(unique(data$BeeID))
length(data$BeeID)
