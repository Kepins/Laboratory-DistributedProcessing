package com.company.service;

import com.company.consumers.ConsumerSandwichCheese;
import com.company.consumers.ConsumerSandwichHam;
import com.company.finisher.Finisher;
import com.company.producers.ProducerBread;
import com.company.producers.ProducerDairy;
import com.company.producers.ProducerHam;
import com.company.products.Bread;
import com.company.products.Butter;
import com.company.products.Cheese;
import com.company.products.Ham;
import com.company.storages.Storage;
import com.company.timeout.Timeout;

import java.util.ArrayList;
import java.util.List;

public class SessionService {
    private SessionParameters sessionParameters;
    private Finisher finisher;

    public Finisher getFinisher() {
        return finisher;
    }

    public SessionService(SessionParameters sessionParameters, Finisher finisher){
        this.sessionParameters = sessionParameters;
        this.finisher = finisher;
    }

    public Thread startGenericSession(){
        Storage<Bread> breads = new Storage<Bread>(sessionParameters.max_capacity_storage);
        Storage<Cheese> cheeses = new Storage<Cheese>(sessionParameters.max_capacity_storage);
        Storage<Butter> butter = new Storage<Butter>(sessionParameters.max_capacity_storage);
        Storage<Ham> hams = new Storage<Ham>(sessionParameters.max_capacity_storage);

        List<Runnable> producers = new ArrayList<>();
        for(int i=0;i<sessionParameters.numberOfProducersBread;i++) {
            producers.add(new ProducerBread(breads));
        }
        for(int i=0;i<sessionParameters.numberOfProducersHam;i++) {
            producers.add(new ProducerHam(hams));
        }
        for(int i=0;i<sessionParameters.numberOfProducersDairy;i++) {
            producers.add(new ProducerDairy(cheeses, butter));
        }
        List<Runnable> consumers = new ArrayList<>();
        for(int i=0;i<sessionParameters.numberOfConsumersSandwichCheese;i++) {
            consumers.add(new ConsumerSandwichCheese(breads, cheeses, butter));
        }
        for(int i=0;i<sessionParameters.numberOfConsumersSandwichHam;i++) {
            consumers.add(new ConsumerSandwichHam(breads, hams, butter));
        }
        List<Thread> threads = new ArrayList<>();
        for(Runnable consumer : consumers){
            threads.add(new Thread(consumer));
        }
        for(Runnable producer : producers){
            threads.add(new Thread(producer));
        }

        Timeout timeout = new Timeout(threads, this.finisher);
        Thread timeoutThread = new Thread(timeout);
        timeoutThread.start();

        for (Thread thread : threads) {
            thread.start();
        }
        return timeoutThread;
    }
}
