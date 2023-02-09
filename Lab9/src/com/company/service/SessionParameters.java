package com.company.service;

public class SessionParameters {
    int numberOfProducersBread ;
    int numberOfProducersDairy ;
    int numberOfProducersHam ;
    int numberOfConsumersSandwichHam;
    int numberOfConsumersSandwichCheese;
    int max_capacity_storage;

    public SessionParameters(int numberOfProducersBread, int numberOfProducersDairy, int numberOfProducersHam, int numberOfConsumersSandwichHam,
                             int numberOfConsumersSandwichCheese, int max_capacity_storage) {
        this.numberOfProducersBread = numberOfProducersBread;
        this.numberOfProducersDairy = numberOfProducersDairy;
        this.numberOfProducersHam = numberOfProducersHam;
        this.numberOfConsumersSandwichHam = numberOfConsumersSandwichHam;
        this.numberOfConsumersSandwichCheese = numberOfConsumersSandwichCheese;
        this.max_capacity_storage = max_capacity_storage;
    }

    public int getNumberOfProducersBread() {
        return numberOfProducersBread;
    }

    public int getNumberOfProducersDairy() {
        return numberOfProducersDairy;
    }

    public int getNumberOfProducersHam() {
        return numberOfProducersHam;
    }

    public int getNumberOfConsumersSandwichHam() {
        return numberOfConsumersSandwichHam;
    }

    public int getNumberOfConsumersSandwichCheese() {
        return numberOfConsumersSandwichCheese;
    }

    public int getMax_capacity_storage() {
        return max_capacity_storage;
    }
}
